#ifndef RTC_DS1307_IMPL_H
#define RTC_DS1307_IMPL_H

#include <Wire.h>
#include <RTClib.h>
#include "IRTCTime.h"
#include "Logger.h"

class RTC_DS1307_Impl : public IRTCTime
{
public:
    RTC_DS1307_Impl(Logger &logger);
    void begin() override;
    void init(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) override;
    String getTime() override;
    String getTimeWithTz() override;
    String getTimeZone() override;
    int8_t getTimeZoneInt() override;
    float getTimeZoneFloat() override;
    void setTimeZone(const String &tz) override;
    void setTimeZone(int8_t tz_h) override;
    void setTimeZone(float fTz) override;
    uint8_t getDate() override;
    uint8_t getHour() override;
    uint8_t getMin() override;
    uint8_t getMonth() override;
    uint8_t getSec() override;
    uint16_t getYear() override;
    uint32_t getUnix() override;

private:
    int16_t _tz;

    RTC_DS1307 _rtc;
    Logger &_logger;
    String _timeZone;

    String _padZero(uint8_t value);
};

#endif
