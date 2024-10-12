#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

enum class LogLevel
{
    NONE,
    PANIC,
    ERROR,
    WARNING,
    INFO,
    DEBUG,
    TRACE,
};

class Logger
{
public:
    Logger(Print &output, const __FlashStringHelper *component, LogLevel minLogLevel = LogLevel::DEBUG)
        : _output(output), _component(component), _minLogLevel(minLogLevel), _enabled(true) {}

    void log(LogLevel level, const __FlashStringHelper *message);
    void log(LogLevel level, const String &message);
    void log(LogLevel level, const char *message);
    void logRaw(int payload);
    void logRaw(const char *payload, size_t length);
    void setMinLogLevel(LogLevel level);
    void enableLogging();
    void disableLogging();
    void printNewLine();

private:
    Print &_output;
    const __FlashStringHelper *_component; // Use __FlashStringHelper* for strings in flash memory
    LogLevel _minLogLevel;
    bool _enabled;
};

#endif
