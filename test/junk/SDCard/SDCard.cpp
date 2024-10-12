#include "SDCard.h"

SDCard::SDCard(Logger &logger, TimeUtils &tim)
    : _logger(logger), _tim(tim)
{
    _cardSize = 0;
    _fileSize = 0;
}

#ifndef DISABLE_SDCARD
SPIClass SPI(SD_MOSI,SD_MISO,SD_SCLK, SD_CS);
void SDCard::init()
{
    SPI.begin();
    if (!SD.begin(SD_CS))
    {
        _logger.log(LogLevel::ERROR, "SDCard mount failed");
    }
    else
    {
        _cardSize = SD.cardSize() / (1024 * 1024);
        _logger.log(LogLevel::INFO, "SDCard mounted, size " + String(_cardSize) + "MB");
    }
}

uint32_t SDCard::sizeMem()
{
    return _cardSize;
}

void SDCard::listDir(const char *dirname, uint8_t levels)
{
    _logger.log(LogLevel::TRACE, "Listing dir: " + String(dirname));

    File root = SD.open(dirname);
    if (!root)
    {
        _logger.log(LogLevel::ERROR, "Failed to open dir");
    }
    if (!root.isDirectory())
    {
        _logger.log(LogLevel::ERROR, "Not a dir");
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            _logger.log(LogLevel::DEBUG, "DIR : " + String(file.name()));
            if (levels)
            {
                listDir(file.name(), levels - 1);
            }
        }
        else
        {
            _logger.log(LogLevel::DEBUG, "FILE : " + String(file.name()) + " SIZE: " + String(file.size()));
        }
        file = root.openNextFile();
    }
}

void SDCard::getLog(String path, String topic, MQTT &mqtt, bool move)
{
    uint8_t levels = 0;

    _logger.log(LogLevel::TRACE, "Listing dir: " + path);
    File root = SD.open(path.c_str(), FILE_READ, true);
    if (!root)
    {
        _logger.log(LogLevel::ERROR, "Failed to open dir");
    }
    if (!root.isDirectory())
    {
        _logger.log(LogLevel::ERROR, "is not dir name");
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            _logger.log(LogLevel::DEBUG, "DIR : " + String(file.name()));
            if (levels)
            {
                listDir(file.name(), levels - 1);
            }
        }
        else
        {
            _logger.log(LogLevel::DEBUG, "FILE : " + String(file.name()) + " SIZE: " + String(file.size()));
            String fname = file.name();
            String msg;
            if (file.available())
            {
                // Serial.write(file.read());
                // sdlog.print("debug", "data SDCard: " + String(file.read()));
                msg = file.readString();

                String dlevel = msg.substring(msg.indexOf("#") + 1, msg.lastIndexOf("#"));
                String dhealth = msg.substring(msg.indexOf("*") + 1, msg.lastIndexOf("*"));
                String dbatt = msg.substring(msg.indexOf("<") + 1, msg.lastIndexOf("<"));
                _logger.log(LogLevel::DEBUG, "dlevel: " + dlevel);
                _logger.log(LogLevel::DEBUG, "dhealth: " + dhealth);
                _logger.log(LogLevel::DEBUG, "dbatt: " + dbatt);

                mqtt.publish(topic + "/log/data", dlevel);
                mqtt.publish(topic + "/log/health", dhealth);
                mqtt.publish(topic + "/log/mppt", dbatt);

                // Remove file if it is published
                if (move==true)
                {   
                    String saveDir = "/save/"+String(_tim.validatedYear())+"/";
                    File file2 = SD.open(saveDir + fname + ".log", FILE_WRITE, true);
                    file2.print(msg);
                    delay(10);
                    SD.remove(path + "/" + fname);
                }
                
            }
            else
            {
                msg = "";
            }
            delay(10);
        }
        file = root.openNextFile();
    }
}

void SDCard::getDebug(String path, String topic, MQTT &mqtt)
{
    uint8_t levels = 0;

    _logger.log(LogLevel::TRACE, "Listing dir: " + path);
    File root = SD.open(path.c_str(), FILE_READ, true);
    if (!root)
    {
        _logger.log(LogLevel::ERROR, "Failed to open dir");
    }
    if (!root.isDirectory())
    {
        _logger.log(LogLevel::ERROR, "is not dir name");
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            _logger.log(LogLevel::DEBUG, "DIR : " + String(file.name()));
            if (levels)
            {
                listDir(file.name(), levels - 1);
            }
        }
        else
        {
            _logger.log(LogLevel::DEBUG, "FILE : " + String(file.name()) + " SIZE: " + String(file.size()));
            String fname = file.name();
            String msg;
            if (file.available())
            {
                // Serial.write(file.read());
                // sdlog.print("debug", "data SDCard: " + String(file.read()));
                msg = file.readString();

                String debuglog = msg.substring(msg.indexOf("#") + 1, msg.lastIndexOf("#\n"));
                _logger.log(LogLevel::DEBUG, "dlevel: " + debuglog);

                mqtt.publish(topic + "/log/debug", debuglog);
                delay(10);
                SD.remove(path + "/" + fname);
            }
            else
            {
                msg = "";
            }
            delay(10);
        }
        file = root.openNextFile();
    }
}

void SDCard::eraselog(String path)
{
    uint8_t levels = 0;

    _logger.log(LogLevel::TRACE, "Listing dir: " + path);

    File root = SD.open(path.c_str(), FILE_READ, true);
    if (!root)
    {
        _logger.log(LogLevel::ERROR, "Failed to open dir");
    }
    if (!root.isDirectory())
    {
        _logger.log(LogLevel::ERROR, "is not dir name");
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            _logger.log(LogLevel::DEBUG, "DIR : " + String(file.name()));
            if (levels)
            {
                listDir(file.name(), levels - 1);
            }
        }
        else
        {
            _logger.log(LogLevel::DEBUG, "FILE : " + String(file.name()) + " SIZE: " + String(file.size()));
            String fname = file.name();
            if (file.available())
            {
                SD.remove(path + "/" + fname);
            }
            delay(10);
        }
        file = root.openNextFile();
    }
}

void SDCard::createDir(const char *path)
{
    _logger.log(LogLevel::TRACE, "Creating Dir: " + String(path));
    if (SD.mkdir(path))
    {
        _logger.log(LogLevel::DEBUG, "Dir created");
    }
    else
    {
        _logger.log(LogLevel::ERROR, "mkdir failed");
    }
}

void SDCard::removeDir(const char *path)
{
    _logger.log(LogLevel::TRACE, "Remove Dir: " + String(path));
    if (SD.rmdir(path))
    {
        _logger.log(LogLevel::DEBUG, "Dir removed");
    }
    else
    {
        _logger.log(LogLevel::ERROR, "rmdir failed");
    }
}

bool SDCard::checkFileExist(const char *path)
{
    _logger.log(LogLevel::TRACE, "Check File Exist: " + String(path));
    if (SD.exists(path))
    {
        _logger.log(LogLevel::DEBUG, "File exists");
        return true;
    }
    else
    {
        _logger.log(LogLevel::DEBUG, "File does not exist");
        return false;
    }
}

String SDCard::readFile(const char *path)
{
    _logger.log(LogLevel::TRACE, "Read File: " + String(path));

    File file = SD.open(path, FILE_READ);
    if (!file)
    {
        _logger.log(LogLevel::ERROR, "Failed to open file for reading");
    }

    _logger.log(LogLevel::DEBUG, "read from file");
    _fileSize = file.size();

    String msg = "";
    if (file.available())
    {
        // Serial.write(file.read());
        // sdlog.print("debug", "data SDCard: " + String(file.read()));
        msg = file.readString();
        _logger.log(LogLevel::DEBUG, "data SDCard: " + msg);
    }

    file.close();

    return msg;
}

size_t SDCard::filelength()
{
    _logger.log(LogLevel::TRACE, "File Length: " + String(_fileSize));
    return _fileSize;
}

String SDCard::readMessage(String path)
{
    return readFile(path.c_str());
}

bool SDCard::writefile(const char *path, String message)
{
    bool write = false;

    File file = SD.open(path, FILE_WRITE, true);
    if (!file)
    {
        _logger.log(LogLevel::ERROR, "Failed to open file for writing");
    }
    if (file.print(message.c_str()))
    {
        _logger.log(LogLevel::DEBUG, "file writen");
        write = true;
    }
    else
    {
        _logger.log(LogLevel::ERROR, "write failed");
    }

    file.close();

    return write;
}

bool SDCard::write(String path, String message)
{
    return writefile(path.c_str(), message);
}
void SDCard::printdebug(LogLevelSD level, String path, String fname, String message)
{
    switch (level)
    {
    case LogLevelSD::NONE:
        write("/debug/" + path + "/" + fname + ".log", "#" + fname + "[NONE]" + message + "#\n");
        break;
    case LogLevelSD::PANIC:
        write("/debug/" + path + "/" + fname + ".log", "#" + fname + "[PANIC]" + message + "#\n");
        break;
    case LogLevelSD::ERROR:
        write("/debug/" + path + "/" + fname + ".log", "#" + fname + "[ERROR]" + message + "#\n");
        break;
    case LogLevelSD::WARNING:
        write("/debug/" + path + "/" + fname + ".log", "#" + fname + "[WARNING]" + message + "#\n");
        break;
    case LogLevelSD::INFO:
        write("/debug/" + path + "/" + fname + ".log", "#" + fname + "[INFO]" + message + "#\n");
        break;
    case LogLevelSD::DEBUG:
        write("/debug/" + path + "/" + fname + ".log", "#" + fname + "[DEBUG]" + message + "#\n");
        break;
    case LogLevelSD::TRACE:
        write("/debug/" + path + "/" + fname + ".log", "#" + fname + "[TRACE]" + message + "#\n");
        break;
    }
}

bool SDCard::writeBytes(const char *path, const uint8_t *buffer, size_t length)
{
    bool write = false;

    File file = SD.open(path, FILE_WRITE, true);
    if (!file)
    {
        _logger.log(LogLevel::ERROR, "Failed to open file for writing");
    }

    size_t written = file.write(buffer, length);
    if (written != length)
    {
        _logger.log(LogLevel::ERROR, "Failed to write data to file");
    }
    else
    {
        write = true;
    }

    file.close();

    return write;
}

bool SDCard::writeFirmware(byte *data, size_t length)
{
    return writeBytes(OTA_DEFAULT_FILE_PATH, data, length);
}

bool SDCard::appendFile(const char *path, const char *message)
{
    bool res = false;
    _logger.log(LogLevel::TRACE, "Appending to file: " + String(path));
    File file = SD.open(path, FILE_APPEND);

    if (!file)
    {
        _logger.log(LogLevel::ERROR, "Failed to open file for appending");
    }

    if (file.print(message))
    {
        _logger.log(LogLevel::DEBUG, "Message appended");
        res = true;
    }
    else
    {
        _logger.log(LogLevel::ERROR, "append failed");
    }

    file.close();

    return res;
}

bool SDCard::appendMessage(String path, String message)
{
    message = message + "#\n";
    return appendFile(path.c_str(), message.c_str());
}

void SDCard::renameFile(const char *path1, const char *path2)
{
    _logger.log(LogLevel::TRACE, "Renaming file: " + String(path1) + "to" + String(path2));

    if (SD.rename(path1, path2))
    {
        _logger.log(LogLevel::DEBUG, "file renamed");
    }
    else
    {
        _logger.log(LogLevel::ERROR, "failed to rename file");
    }
}

void SDCard::deleteFile(const char *path)
{
    _logger.log(LogLevel::TRACE, "Deleting file: " + String(path));
    if (SD.remove(path))
    {
        _logger.log(LogLevel::DEBUG, "file deleted");
    }
    else
    {
        _logger.log(LogLevel::ERROR, "delete failed");
    }
}

bool SDCard::copyFile(const char *path1, const char *path2)
{
    bool copied;
    String msg;

    File file = SD.open(path1, FILE_READ);
    if (!file)
    {
        _logger.log(LogLevel::ERROR, "Failed to open file for reading");
    }

    _logger.log(LogLevel::DEBUG, "read from file");
    _fileSize = file.size();

    if (file.available())
    {
        // Serial.write(file.read());
        // sdlog.print("debug", "data SDCard: " + String(file.read()));

        msg = file.readString();
        copied = true;
    }
    else
    {
        msg = "";
        copied = false;
    }

    File file2 = SD.open(path2, FILE_WRITE, true);
    if (!file2)
    {
        _logger.log(LogLevel::ERROR, "Failed to open file for writing");
        copied = false;
    }
    if (file2.print(msg.c_str()))
    {
        _logger.log(LogLevel::DEBUG, "file writen");
        copied = true;
    }
    else
    {
        _logger.log(LogLevel::ERROR, "write failed");
        copied = false;
    }
    file2.close();

    return copied;
}

float SDCard::getTotalSpace()
{
    return SD.totalBytes() / (1024 * 1024);
}

float SDCard::getUsedSpace()
{
    return SD.usedBytes() / (1024 * 1024);
}

#else

void SDCard::init()
{
}

uint32_t SDCard::sizeMem()
{
    return 0;
}

void SDCard::listDir(const char *dirname, uint8_t levels)
{
}

void SDCard::getLog(String path, String topic, MQTT &mqtt)
{
}

void SDCard::getDebug(String path, String topic, MQTT &mqtt)
{
}

void SDCard::eraselog(String path)
{
}

void SDCard::createDir(const char *path)
{
}

void SDCard::removeDir(const char *path)
{
}

bool SDCard::checkFileExist(const char *path)
{
    return false;
}

String SDCard::readFile(const char *path)
{
    return "";
}

size_t SDCard::filelength()
{
    return 0;
}

String SDCard::readMessage(String path)
{
    return "";
}

bool SDCard::writefile(const char *path, String message)
{
    return false;
}

bool SDCard::write(String path, String message)
{
    return false;
}

void SDCard::printdebug(LogLevelSD level, String path, String fname, String message)
{
}

bool SDCard::writeBytes(const char *path, const uint8_t *buffer, size_t length)
{
    return false;
}

bool SDCard::writeFirmware(byte *data, size_t length)
{
    return false;
}

bool SDCard::appendFile(const char *path, const char *message)
{
    return false;
}

bool SDCard::appendMessage(String path, String message)
{
    return false;
}

void SDCard::renameFile(const char *path1, const char *path2)
{
}

void SDCard::deleteFile(const char *path)
{
}

bool SDCard::copyFile(const char *path1, const char *path2)
{
    return false;
}

float SDCard::getTotalSpace()
{
    return 0;
}

float SDCard::getUsedSpace()
{
    return 0;
}

#endif