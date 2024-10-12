#include "Logger.h"

void Logger::log(LogLevel level, const char *message)
{
    if (!_enabled || level > _minLogLevel)
    {
        return;
    }
    switch (level)
    {
    case LogLevel::TRACE:
        _output.print(F("[TRACE]"));
        break;
    case LogLevel::DEBUG:
        _output.print(F("[DEBUG]"));
        break;
    case LogLevel::INFO:
        _output.print(F("[INFO]"));
        break;
    case LogLevel::WARNING:
        _output.print(F("[WARNING]"));
        break;
    case LogLevel::ERROR:
        _output.print(F("[ERROR]"));
        break;
    case LogLevel::PANIC:
        _output.print(F("[PANIC]"));
        break;
    }
    _output.print(F("["));
    _output.print(_component);
    _output.print(F("] "));
    _output.println(message);
}

void Logger::log(LogLevel level, const __FlashStringHelper *message)
{
    log(level, reinterpret_cast<const char *>(message));
}

void Logger::log(LogLevel level, const String &message)
{
    log(level, message.c_str());
}

void Logger::logRaw(int payload)
{
    _output.write(payload);
}

void Logger::logRaw(const char *payload, size_t length)
{
    _output.write(payload, length);
}

void Logger::setMinLogLevel(LogLevel level)
{
    _minLogLevel = level;
}

void Logger::enableLogging()
{
    _enabled = true;
}

void Logger::disableLogging()
{
    _enabled = false;
}

void Logger::printNewLine()
{
    _output.println();
}
