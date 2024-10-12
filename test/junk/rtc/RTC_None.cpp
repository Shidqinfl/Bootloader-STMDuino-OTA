#include "RTC_None.h"
#include "Logger.h"

RTC_None::RTC_None(Logger &logger) : _logger(logger)
{
    _timeZone = "";
}

void RTC_None::begin()
{
}

void RTC_None::init(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second)
{
}

String RTC_None::getTime()
{
    // 1970-01-01T00:00:00
    char buffer[20];
    sprintf(buffer, "%4d-%02d-%02dT%02d:%02d:%02d", 1970, 1, 1, 0, 0, 0);
    return String(buffer);
}

String RTC_None::getTimeWithTz()
{
    return String(1970) + "-" + _padZero(10) + "-" + _padZero(01) + "T" +
           _padZero(0) + ":" + _padZero(0) + ":" + _padZero(0) + getTimeZone();
}

String RTC_None::getTimeZone()
{
    return _timeZone;
}

int8_t RTC_None::getTimeZoneInt()
{
    return (int)_RTimeZone;
}

float RTC_None::getTimeZoneFloat()
{
    return _RTimeZone;
}

void RTC_None::setTimeZone(const String &tz)
{
    _timeZone = tz;
}

void RTC_None::setTimeZone(int8_t tz_h)
{
    // Create string from tz_h value, format is "+tz_h:00" or "-tz_h:00"
    char tzStr[10];
    memset(tzStr, 0, sizeof(tzStr));
    sprintf(tzStr, "%s%02d:00", tz_h >= 0 ? "+" : "-", abs(tz_h));
    _timeZone = String(tzStr);
}

void RTC_None::setTimeZone(float fTz)
{
    int tz_h = (int)fTz;               // get the hour part
    int tz_m = abs((fTz - tz_h) * 60); // get the minute part
    char tzStr[10];
    memset(tzStr, 0, sizeof(tzStr));
    sprintf(tzStr, "%s%02d:%02d", fTz >= 0 ? "+" : "-", abs(tz_h), tz_m);
    _timeZone = String(tzStr);
}

uint8_t RTC_None::getDate()
{
    return 1;
}

uint8_t RTC_None::getHour()
{
    return 0;
}

uint8_t RTC_None::getMin()
{
    return 0;
}

uint8_t RTC_None::getSec()
{
    return 0;
}

uint8_t RTC_None::getMonth()
{
    return 1;
}

uint16_t RTC_None::getYear()
{
    return 1;
}

uint32_t RTC_None::getUnix()
{
    return 0;
}

String RTC_None::_padZero(uint8_t value)
{
    if (value < 10)
    {
        return "0" + String(value);
    }
    return String(value);
}
