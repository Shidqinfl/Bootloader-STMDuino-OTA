#include "RTC_DS1307_Impl.h"
#include "Logger.h"

RTC_DS1307_Impl::RTC_DS1307_Impl(Logger &logger) : _rtc(), _logger(logger)
{
    _timeZone = "";
}

void RTC_DS1307_Impl::begin()
{
    _rtc.begin();
    if (!_rtc.isrunning())
    {
        _logger.log(LogLevel::ERROR, F("RTC is NOT running!"));
    }
}

void RTC_DS1307_Impl::init(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second)
{
    _rtc.adjust(DateTime(year, month, day, hour, minute, second));
}

bool invalidateDate(uint16_t year, uint8_t month, uint8_t day)
{
    return month == 0 || month > 12 || day == 0 || day > 31;
    // return (year > 2000 && year < 3000) && (month >= 0 && month <= 12) && (day > 0 && day <= 31);
}

bool invalidateTime(uint8_t hour, uint8_t minute, uint8_t second)
{
    // sudah unsigned, tidak perlu cek negatif
    return hour > 23 || minute > 59 || second > 59;
}

String stringEmpty = "";

String RTC_DS1307_Impl::getTime()
{
    String result;
    DateTime now = _rtc.now();
    _RYear = now.year();
    _RMonth = now.month();
    _RDay = now.day();
    _RHour = now.hour();
    _RMin = now.minute();
    _RSec = now.second();

    if (invalidateDate(_RYear, _RMonth, _RDay) || invalidateTime(_RHour, _RMin, _RSec))
    {
        Serial.println("[RTC][ERROR] invalid time");
        return stringEmpty;
    }

    char buffer[24];
    sprintf(buffer, "%4d-%02d-%02dT%02d:%02d:%02d", _RYear, _RMonth, _RDay, _RHour, _RMin, _RSec);
    return String(buffer);
}

String RTC_DS1307_Impl::getTimeWithTz()
{
    String result;
    DateTime now = _rtc.now();
    _RYear = now.year();
    _RMonth = now.month();
    _RDay = now.day();
    _RHour = now.hour();
    _RMin = now.minute();
    _RSec = now.second();

    if (invalidateDate(_RYear, _RMonth, _RDay) || invalidateTime(_RHour, _RMin, _RSec))
    {
        Serial.println("[RTC][ERROR] invalid time");
        return stringEmpty;
    }

    char buffer[32];
    // char tzSign = _TZ '+';

    sprintf(buffer, "%4d-%02d-%02dT%02d:%02d:%02d%s",
            _RYear, _RMonth, _RDay, _RHour, _RMin, _RSec, _timeZone.c_str());
    return String(buffer);
}

String RTC_DS1307_Impl::getTimeZone()
{
    return _timeZone;
}

int8_t RTC_DS1307_Impl::getTimeZoneInt()
{
    return (int)_RTimeZone;
}

float RTC_DS1307_Impl::getTimeZoneFloat()
{
    return _RTimeZone;
}

void RTC_DS1307_Impl::setTimeZone(const String &tz)
{
    _timeZone = tz;
}

void RTC_DS1307_Impl::setTimeZone(int8_t tz_h)
{
    // Create string from tz_h value, format is "+tz_h:00" or "-tz_h:00"
    char tzStr[10];
    memset(tzStr, 0, sizeof(tzStr));
    sprintf(tzStr, "%s%02d:00", tz_h >= 0 ? "+" : "-", abs(tz_h));
    _timeZone = String(tzStr);
    _RTimeZone = tz_h;
}

void RTC_DS1307_Impl::setTimeZone(float fTz)
{
    _RTimeZone = fTz;
    int tz_h = (int)fTz;               // get the hour part
    int tz_m = abs((fTz - tz_h) * 60); // get the minute part
    char tzStr[10];
    memset(tzStr, 0, sizeof(tzStr));
    sprintf(tzStr, "%s%02d:%02d", fTz >= 0 ? "+" : "-", abs(tz_h), tz_m);
    _timeZone = String(tzStr);
}

uint8_t RTC_DS1307_Impl::getDate()
{
    return _RDay;
}

uint8_t RTC_DS1307_Impl::getMonth()
{
    return _RMonth;
}

uint8_t RTC_DS1307_Impl::getMin()
{
    return _RMin;
}

uint8_t RTC_DS1307_Impl::getSec()
{
    return _RSec;
}

uint8_t RTC_DS1307_Impl::getHour()
{
    return _RHour;
}

uint16_t RTC_DS1307_Impl::getYear()
{
    return _RYear;
}

uint32_t RTC_DS1307_Impl::getUnix()
{
    return _rtc.now().unixtime();
}

String RTC_DS1307_Impl::_padZero(uint8_t value)
{
    if (value < 10)
    {
        return "0" + String(value);
    }
    return String(value);
}
