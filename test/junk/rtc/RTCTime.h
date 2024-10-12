#ifndef RTCTIME_H
#define RTCTIME_H

#include "IRTCTime.h"
#include "Logger.h"

class RTCTime
{
public:
    RTCTime(IRTCTime &rtc) : _rtc(rtc) {}

    void begin()
    {
        _rtc.begin();
    }

    void init(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second)
    {
        _rtc.init(year, month, day, hour, minute, second);
    }

    String getTime()
    {
        return _rtc.getTime();
    }

    String getTimeWithTz()
    {
        return _rtc.getTimeWithTz();
    }

    String getTimeZone()
    {
        return _rtc.getTimeZone();
    }

    int8_t getTimeZoneInt()
    {
        return _rtc.getTimeZoneInt();
    }

    float getTimeZoneFloat()
    {
        return _rtc.getTimeZoneFloat();
    }

    void setTimeZone(const String &tz)
    {
        _rtc.setTimeZone(tz);
    }

    void setTimeZone(int8_t tz_h)
    {
        _rtc.setTimeZone(tz_h);
    }

    void setTimeZone(float fTz)
    {
        _rtc.setTimeZone(fTz);
    }

    uint8_t getDate()
    {
        return _rtc.getDate();
    }

    uint8_t getHour()
    {
        return _rtc.getHour();
    }
    uint8_t getMonth()
    {
        return _rtc.getMonth();
    }
    uint8_t getMin()
    {
        return _rtc.getMin();
    }
    uint8_t getSec()
    {
        return _rtc.getSec();
    }
    uint16_t getYear()
    {
        return _rtc.getYear();
    }

    uint32_t getUnix()
    {
        return _rtc.getUnix();
    }

private:
    IRTCTime &_rtc;
};

#endif
