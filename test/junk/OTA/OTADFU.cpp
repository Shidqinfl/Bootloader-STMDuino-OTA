#include "OTADFU.h"
#include "Logger.h"
#include "Configurator.h"
#include <SPI.h>
#include <SD.h>
#include <SPIFFS.h>

OTADFU::OTADFU(TinyGsmClient &client, Logger &logger) : _client(client), _logger(logger)
{
    _fsys = nullptr;
    _otaUrl = "http://monitoringku.com/update.bin";
    _printPercentageCallback = nullptr;
}

void OTADFU::init()
{
    init(OTA_FS_AUTO);
}

void OTADFU::init(uint8_t fs)
{
#ifdef DISABLE_SDCARD
    _logger.log(LogLevel::INFO, "Mounting SPIFFS, format when failed");
    if (SPIFFS.begin(true))
    {
        _logger.log(LogLevel::INFO, "SPIFFS mounted");
    }
    else
    {
        _logger.log(LogLevel::ERROR, "SPIFFS mount failed");
    }
    _fsys = &SPIFFS;

#else
    if (fs == OTA_FS_SD)
    {
        // Check if SD card is available, use it as primary storage
        SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
        if (SD.begin(SD_CS))
        {
            _fsys = &SD;
        }
        else
        {
            _logger.log(LogLevel::ERROR, "SD card mount failed");
        }
    }
    else if (fs == OTA_FS_SPIFFS)
    {
        _logger.log(LogLevel::INFO, "Mounting SPIFFS, format when failed");
        if (SPIFFS.begin(true))
        {
            _logger.log(LogLevel::INFO, "SPIFFS mounted");
            _fsys = &SPIFFS;
        }
        else
        {
            _logger.log(LogLevel::ERROR, "SPIFFS mount failed");
        }
    }
    else
    {
        // Check if SD card is available, use it as primary storage
        SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
        if (SD.begin(SD_CS))
        {
            _fsys = &SD;
        }
        else
        {
            _logger.log(LogLevel::INFO, "Mounting SPIFFS, format when failed");
            if (SPIFFS.begin(true))
            {
                _logger.log(LogLevel::INFO, "SPIFFS mounted");
                _fsys = &SPIFFS;
            }
            else
            {
                _logger.log(LogLevel::ERROR, "SPIFFS mount failed");
            }
        }
    }
#endif
}

void OTADFU::setOTAUrl(String url)
{
    _otaUrl = url;
}

String OTADFU::getOTAUrl()
{
    return _otaUrl;
}

bool OTADFU::downloadFromUrl(bool chunked)
{
    if (!_fsys)
    {
        _logger.log(LogLevel::ERROR, "File system not initialized");
        return false;
    }

    String protocol, server, path;
    uint16_t port;
    _parseUrl(_otaUrl, protocol, server, port, path);
    server.trim();
    protocol.trim();
    path.trim();

    _logger.log(LogLevel::INFO, "Connecting to server: " + server + " on port: " + String(port) + " with path: " + path);

    // Download data
    String contentType;
    uint32_t contentLength = 0;
    uint32_t startRange = 0;
    uint32_t endRange = 0;
    uint32_t totalLength = 0;
    uint32_t readLength = 0;
    const uint32_t chunkSize = HTTP_CHUNK_SIZE_BYTES;
    uint32_t clientReadStartTime = millis();

    // Removing old file
    if (!_fsys->remove(OTA_DEFAULT_FILE_PATH))
    {
        _logger.log(LogLevel::ERROR, "Error removing old file or not exist, continue");
    }

    File updateBin = _fsys->open(OTA_DEFAULT_FILE_PATH, FILE_WRITE);
    if (!updateBin)
    {
        _logger.log(LogLevel::ERROR, "Error opening file for writing");
        return false;
    }

    do
    {
        // Send request to server
        if (!_connectToServer(server, port, path, startRange, endRange))
        {
            _logger.log(LogLevel::ERROR, "Enough retrying connect to server");
            _client.stop();
            return false;
        }

        // Get response
        if (!_readHeader(contentType, contentLength))
        {
            _logger.log(LogLevel::ERROR, "Error reading header");
            _client.stop();
            return false;
        }

        // Check content type
        if (!contentType.equals(F("application/octet-stream")))
        {
            _logger.log(LogLevel::ERROR, "Invalid content type: " + contentType);
            _client.stop();
            return false;
        }

        // Read data in chunk, initialize with total length data
        if (totalLength == 0)
        {
            _logger.log(LogLevel::INFO, "Content length: " + String(contentLength) + " bytes");
            _logger.log(LogLevel::INFO, "Content type: " + contentType);
            totalLength = contentLength;
            startRange = 0;

            if (!chunked)
            {
                // Download at once
                while (_client.available())
                {
                    int c = _client.read();
                    if (c == -1)
                    {
                        _logger.log(LogLevel::ERROR, "Error reading from server");
                        updateBin.close();
                        _client.stop();
                        return false;
                    }
                    else
                    {
                        readLength++;
                        updateBin.write((uint8_t)c);

                        if (readLength % (totalLength / 13) == 0)
                        {
                            _printPercent(readLength, totalLength);
                        }
                    }
                }
                break;
            }
        }
        else
        {
            // Start reading data
            for (int i = startRange; i <= endRange; i++)
            {
                int c = _client.read();
                if (c == -1)
                {
                    if (readLength != totalLength)
                    {
                        _logger.log(LogLevel::ERROR, "Error reading from server");
                        updateBin.close();
                        _client.stop();

                        return false;
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    readLength++;
                    updateBin.write((uint8_t)c);

                    if (readLength % (totalLength / 13) == 0)
                    {
                        _printPercent(readLength, totalLength);
                    }
                }
                clientReadStartTime = millis();
            }

            // Next chunck
            startRange += chunkSize;
        }

        endRange = startRange + chunkSize - 1;

        if (endRange >= totalLength)
        {
            endRange = totalLength;
        }

        if (startRange >= totalLength)
        {
            startRange = totalLength;
        }

    } while (readLength < totalLength && millis() - clientReadStartTime < HTTP_REQUEST_TIMEOUT_MS);

    updateBin.close();
    _logger.log(LogLevel::INFO, "Total read: " + String(readLength) + " bytes");
    _logger.log(LogLevel::INFO, "Total length: " + String(totalLength) + " bytes");
    if (readLength != totalLength)
    {
        _logger.log(LogLevel::ERROR, "Download incomplete");

        // Remove incomplete file
        if (!_fsys->remove(OTA_DEFAULT_FILE_PATH))
        {
            _logger.log(LogLevel::ERROR, "Error removing old file or not exist, continue");
        }
    }
    else
    {
        _logger.log(LogLevel::INFO, "Download complete");
        _client.stop();
        return true;
    }

    return false;
}

// check given FS for valid firmware and perform update if available
bool OTADFU::updateFromFS()
{
    if (!_fsys)
    {
        _logger.log(LogLevel::ERROR, "File system not initialized");
        return false;
    }

    File updateBin = _fsys->open(OTA_DEFAULT_FILE_PATH);
    bool res = false;

    if (updateBin)
    {
        if (updateBin.isDirectory())
        {
            _logger.log(LogLevel::ERROR, "is not a file");
            updateBin.close();
            return false;
        }

        size_t updateSize = updateBin.size();

        if (updateSize > 0)
        {
            _logger.log(LogLevel::INFO, "Try to start update");
            res = _performUpdate(updateBin, updateSize);
        }
        else
        {
            _logger.log(LogLevel::ERROR, "File is empty");
        }

        updateBin.close();

        // whe finished remove the binary from sd card to indicate end of the process
        _fsys->remove(OTA_DEFAULT_FILE_PATH);
    }
    else
    {
        _logger.log(LogLevel::ERROR, "Could not load firmware from sd root");
    }

    return res;
}

void OTADFU::setPrintPercentageCallback(PrintPercentageCallback callback)
{
    _printPercentageCallback = callback;
}

bool OTADFU::_performUpdate(Stream &updateSource, size_t updateSize)
{
    bool res = false;
    if (Update.begin(updateSize))
    {
        size_t written = Update.writeStream(updateSource);
        if (written == updateSize)
        {
            _logger.log(LogLevel::INFO, "Written : " + String(written) + " successfully");
        }
        else
        {
            _logger.log(LogLevel::ERROR, "Written only : " + String(written) + "/" + String(updateSize) + ". Retry?");
        }
        if (Update.end())
        {
            _logger.log(LogLevel::INFO, "OTA done!");
            if (Update.isFinished())
            {
                _logger.log(LogLevel::INFO, "Update successfully completed. Rebooting.");
                res = true;
            }
            else
            {
                _logger.log(LogLevel::ERROR, "Update not finished? Something went wrong!");
            }
        }
        else
        {
            _logger.log(LogLevel::ERROR, "Error Occurred. Error #: " + String(Update.getError()));
        }
    }
    else
    {
        _logger.log(LogLevel::ERROR, "Not enough space to begin OTA");
    }

    return res;
}

void OTADFU::_parseUrl(const String &url, String &protocol, String &server, uint16_t &port, String &path)
{
    // Find the position of the colon (:) after the protocol (http:// or https://)
    int colonIndex = url.indexOf(':');

    // Extract protocol
    protocol = url.substring(0, colonIndex);

    // Find the position of the slash (/) after the protocol
    int doubleSlashIndex = url.indexOf("//");
    int slashIndex = url.indexOf('/', doubleSlashIndex + 2);

    // Extract server and port
    if (colonIndex != -1 && slashIndex != -1)
    {
        String serverPort = url.substring(doubleSlashIndex + 2, slashIndex); // Extract server and port
        int portSeparatorIndex = serverPort.indexOf(':');                    // Find the position of the colon (:) that separates server and port
        if (portSeparatorIndex != -1)
        {
            server = serverPort.substring(0, portSeparatorIndex);        // Extract server
            port = serverPort.substring(portSeparatorIndex + 1).toInt(); // Extract port and convert to integer
        }
        else
        {
            server = serverPort;
            port = protocol == "http" ? 80 : 443; // Default HTTP or HTTPS port
        }
    }
    else
    {
        // Default port if not specified in URL
        server = url.substring(doubleSlashIndex + 2); // Extract server
        port = protocol == "http" ? 80 : 443;         // Default HTTP or HTTPS port
    }

    // Extract path
    path = url.substring(slashIndex); // Extract path
}

void OTADFU::_printPercent(uint32_t readLength, uint32_t contentLength)
{
    // If we know the total length
    float percent = (100.0 * readLength) / contentLength;
    if (contentLength != (uint32_t)-1)
    {
        _logger.log(LogLevel::INFO, "Progress: " + String(percent) + "%");
    }
    else
    {
        _logger.log(LogLevel::INFO, "Received: " + String(readLength) + " bytes");
    }

    if (_printPercentageCallback)
    {
        _printPercentageCallback(percent);
    }
}

bool OTADFU::_readHeader(String &contentType, uint32_t &contentLength)
{
    uint32_t clientReadStartTime = millis();
    String headerBuffer;
    bool finishedHeader = false;

    // Read header
    while (!finishedHeader)
    {
        int nlPos;

        if (_client.available())
        {
            clientReadStartTime = millis();
            while (_client.available())
            {
                char c = _client.read();
                headerBuffer += c;
                if (headerBuffer.indexOf(F("\r\n")) >= 0)
                    break;
            }
        }
        else
        {
            if (millis() - clientReadStartTime > HTTP_REQUEST_TIMEOUT_MS)
            {
                // Time-out waiting for data from client
                _logger.log(LogLevel::ERROR, "Connection timeout");
                break;
            }
        }

        // See if we have a new line.
        nlPos = headerBuffer.indexOf(F("\r\n"));

        if (nlPos > 0)
        {
            headerBuffer.toLowerCase();
            if (headerBuffer.startsWith(F("content-length:")))
            {
                contentLength =
                    headerBuffer.substring(headerBuffer.indexOf(':') + 1).toInt();
            }
            else if (headerBuffer.startsWith(F("content-type:")))
            {
                contentType = headerBuffer.substring(headerBuffer.indexOf(':') + 1);
                contentType.trim();
            }
            headerBuffer.remove(0, nlPos + 2);
        }
        else if (nlPos == 0)
        {
            finishedHeader = true;
        }
    }

    return finishedHeader;
}

bool OTADFU::_connectToServer(String server, uint16_t port, String path, uint32_t startRange, uint32_t endRange)
{
    uint8_t retry = 0;

    while (retry < HTTP_REQUEST_MAX_RETRY)
    {
        _client.flush();
        if (_client.connect(server.c_str(), port))
        {
            String header = String("GET ") + path + " HTTP/1.1\r\n";
            header += String("Host: ") + server + "\r\n";
            if (startRange != 0 && endRange != 0)
            {
                header += "Range: bytes=" + String(startRange) + "-" + String(endRange) + "\r\n";
            }
            header += "Connection: close\r\n\r\n";
            _client.print(header);

            return true;
        }
        retry++;
        delay(HTTP_REQUEST_RETRY_DELAY_MS);
    }

    return false;
}

// OTADriveDFU
OTADriveDFU::OTADriveDFU(TinyGsmClient &client, Logger &logger) : _client(client), _logger(logger)
{
    _onProgressUpdateCallback = nullptr;
    _firstRun = true;
    _fsys = nullptr;
}

void OTADriveDFU::init(String apiKey, String fwVersion, uint8_t fs)
{
    OTADRIVE.setInfo(apiKey, fwVersion);

#ifdef DISABLE_SDCARD
    _logger.log(LogLevel::INFO, "Mounting SPIFFS, format when failed");
    if (SPIFFS.begin(true))
    {
        _logger.log(LogLevel::INFO, "SPIFFS mounted");
    }
    else
    {
        _logger.log(LogLevel::ERROR, "SPIFFS mount failed");
    }
    _fsys = &SPIFFS;

#else
    if (fs == OTA_FS_SD)
    {
        // Check if SD card is available, use it as primary storage
        SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
        if (SD.begin(SD_CS))
        {
            _fsys = &SD;
        }
        else
        {
            _logger.log(LogLevel::ERROR, "SD card mount failed");
        }
    }
    else if (fs == OTA_FS_SPIFFS)
    {
        _logger.log(LogLevel::INFO, "Mounting SPIFFS, format when failed");
        if (SPIFFS.begin(true))
        {
            _logger.log(LogLevel::INFO, "SPIFFS mounted");
            _fsys = &SPIFFS;
        }
        else
        {
            _logger.log(LogLevel::ERROR, "SPIFFS mount failed");
        }
    }
    else
    {
        // Check if SD card is available, use it as primary storage
        SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
        if (SD.begin(SD_CS))
        {
            _fsys = &SD;
        }
        else
        {
            _logger.log(LogLevel::INFO, "Mounting SPIFFS, format when failed");
            if (SPIFFS.begin(true))
            {
                _logger.log(LogLevel::INFO, "SPIFFS mounted");
                _fsys = &SPIFFS;
            }
            else
            {
                _logger.log(LogLevel::ERROR, "SPIFFS mount failed");
            }
        }
    }
#endif

    OTADRIVE.setFileSystem(_fsys);
}

void OTADriveDFU::init()
{
    init(OTA_DRIVE_APIKEY, FW_VERSION, OTA_FS_AUTO);
}

void OTADriveDFU::init(String apiKey, uint8_t fs)
{
    init(apiKey, FW_VERSION, fs);
}

void OTADriveDFU::setBaseURL(String url)
{
    OTADRIVE.setBaseURL(url);
}

String OTADriveDFU::getBaseURL()
{
    return OTADRIVE.getBaseURL();
}

void OTADriveDFU::setOnProgressUpdateCallback(OnProgressUpdateCallback callback)
{
    OTADRIVE.onUpdateFirmwareProgress(callback);
}

bool OTADriveDFU::forceUpdate()
{
    FotaResult res = OTADRIVE.updateFirmware(_client, false);

    if (res == FOTA_UPDATE_OK)
    {
        return true;
    }

    return false;
}

void OTADriveDFU::loop()
{
    if (OTADRIVE.timeTick(OTA_DRIVE_CHECK_INTERVAL_S))
    {
        if (!_firstRun)
        {
            _logger.log(LogLevel::INFO, "Checking for updates -- blocking");

   

            FotaResult info = OTADRIVE.updateFirmware(_client, false);
            if (info == FOTA_UPDATE_OK)
            {
                _logger.log(LogLevel::INFO, "Update successful");
            }
            else if (info == FOTA_UPDATE_FAILED)
            {
                _logger.log(LogLevel::ERROR, "Update failed");
            }
            else if (info == FOTA_UPDATE_NO_UPDATES)
            {
                _logger.log(LogLevel::INFO, "No updates available");
            }
        }
        else
        {
            _firstRun = false;
        }
    }
}