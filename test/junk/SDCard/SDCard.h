#ifndef SDCARD_H
#define SDCARD_H

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "configurator.h"
#include "Logger.h"
#include "MQTT.h"
#include "timeUtils.h"

enum class LogLevelSD
{
    NONE,
    PANIC,
    ERROR,
    WARNING,
    INFO,
    DEBUG,
    TRACE,
};

class SDCard
{
public:
    SDCard(Logger &logger, TimeUtils &tim);

    void init();
    uint32_t sizeMem();
    void listDir(const char *dirname, uint8_t levels);
    void createDir(const char *path);
    void removeDir(const char *path);

    bool checkFileExist(const char *path);
    String readFile(const char *path);
    size_t filelength();
    String readMessage(String path);
    bool writefile(const char *path, String message);
    bool write(String path, String message);
    bool writeBytes(const char *path, const uint8_t *buffer, size_t length);
    bool writeFirmware(byte *data, size_t length);

    bool appendFile(const char *path, const char *message);
    bool appendMessage(String path, String message);
    void renameFile(const char *path1, const char *path2);
    void deleteFile(const char *path);
    bool copyFile(const char *path1, const char *path2);

    void getLog(String path, String topic, MQTT &mqtt, bool move);
    void eraselog(String path);

    void getDebug(String path, String topic, MQTT &mqtt);
    void printdebug(LogLevelSD level, String path, String fname, String message);

    float getTotalSpace();
    float getUsedSpace();

private:
    Logger &_logger; 
    TimeUtils &_tim;
    uint32_t _cardSize;
    size_t _fileSize;
};

#endif