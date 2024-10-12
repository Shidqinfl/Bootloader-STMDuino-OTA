#ifndef RTC_None_H
#define RTC_None_H

#include "IRTCTime.h"
#include "Logger.h"

class RTC_None : public IRTCTime
{
public:
    RTC_None(Logger &logger);
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
    uint8_t getSec() override;
    uint8_t getMonth() override;
    uint16_t getYear() override;
    uint32_t getUnix() override;

private:
    Logger &_logger;
    String _timeZone;

    String _padZero(uint8_t value);
};

#endif
