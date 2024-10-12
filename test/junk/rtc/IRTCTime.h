#ifndef IRTCTIME_H
#define IRTCTIME_H

#include <Arduino.h>

class IRTCTime
{
public:
    virtual void begin() = 0;
    virtual void init(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) = 0;
    virtual String getTime() = 0;
    virtual String getTimeWithTz() = 0;
    virtual String getTimeZone() = 0;
    virtual int8_t getTimeZoneInt() = 0;
    virtual float getTimeZoneFloat() = 0;
    virtual void setTimeZone(const String &tz) = 0;
    virtual void setTimeZone(int8_t tz_h) = 0;
    virtual void setTimeZone(float fTz) = 0;
    virtual uint8_t getDate() = 0;
    virtual uint8_t getMonth() = 0;
    virtual uint16_t getYear() = 0;
    virtual uint8_t getHour() = 0;
    virtual uint8_t getMin() = 0;
    virtual uint8_t getSec() = 0;

    virtual uint32_t getUnix() = 0;

    uint16_t _RYear;
    uint8_t _RMonth;
    uint8_t _RDay;
    uint8_t _RHour;
    uint8_t _RMin;
    uint8_t _RSec;
    float _RTimeZone;
};

#endif
