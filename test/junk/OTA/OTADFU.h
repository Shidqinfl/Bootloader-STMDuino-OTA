#ifndef OTA_DFU_H
#define OTA_DFU_H

#include <Arduino.h>
// #include <Update.h>
// #include <FS.h>
#include "Configurator.h"
#include "Logger.h"
#include <TinyGsmClient.h>
#include "OTADrive/otadrive_esp.h"


enum OTA_INFO : int8_t
{
    OTA_INFO_ERROR = -1,
    OTA_INFO_CMD_MQTT = 0,
    OTA_INFO_DOWNLOADED = 1,
    OTA_INFO_SUCCESS = 2
};

enum OTA_FS : uint8_t
{
    OTA_FS_AUTO = 0,
    OTA_FS_SPIFFS = 1,
    OTA_FS_SD = 2
};

typedef void (*PrintPercentageCallback)(float);
typedef void (*OnProgressUpdateCallback)(size_t, size_t);

class OTADFU
{
public:
    OTADFU(TinyGsmClient &client, Logger &logger);
    void init();
    void init(uint8_t fs);
    void setOTAUrl(String url);
    String getOTAUrl();
    bool downloadFromUrl(bool chunked);
    bool updateFromFS();
    void setPrintPercentageCallback(PrintPercentageCallback callback);

private:
    TinyGsmClient &_client;
    Logger &_logger;
    String _otaUrl;
    fs::FS *_fsys;
    PrintPercentageCallback _printPercentageCallback;
    void _parseUrl(const String &url, String &protocol, String &server, uint16_t &port, String &path);
    bool _performUpdate(Stream &updateSource, size_t updateSize);
    void _printPercent(uint32_t readLength, uint32_t contentLength);
    bool _connectToServer(String server, uint16_t port, String path, uint32_t startRange, uint32_t endRange);
    bool _readHeader(String &contentType, uint32_t &contentLength);
};

class OTADriveDFU
{
public:
    OTADriveDFU(TinyGsmClient &client, Logger &logger);
    void init();
    void init(String apiKey, uint8_t fs);
    void init(String apiKey, String fwVersion, uint8_t fs);
    void setBaseURL(String url);
    String getBaseURL();
    void setOnProgressUpdateCallback(OnProgressUpdateCallback callback);
    bool forceUpdate();
    void loop();

private:
    TinyGsmClient &_client;
    Logger &_logger;
    bool _firstRun;
    fs::FS *_fsys;
    OnProgressUpdateCallback _onProgressUpdateCallback;
};
#endif // OTA_DFU_H