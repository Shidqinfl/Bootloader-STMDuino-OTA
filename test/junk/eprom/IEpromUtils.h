#ifndef IEPROMUTILS_H
#define IEPROMUTILS_H
#include "Configurator.h"

class IEprom
{   
    public:
    std::string broker[4] = {"", "", "", ""};
    virtual void begin() = 0;
    virtual void clearAll() = 0;
    virtual bool validate() = 0;
    virtual void writeBrokerStr(const std::string bconf[4]) = 0;
    virtual void readBrokerStr() = 0;
    virtual uint8_t readByte(uint16_t address) = 0;
    virtual void writeByte(uint16_t address, uint8_t data) = 0;
    virtual String readModelAC() = 0;
    virtual void writeModelAC(const String& model) = 0;
    virtual uint8_t readIntervalSend() = 0;
    virtual void writeIntervalSend(uint8_t interval) = 0;
    virtual uint8_t readIntervalReboot() = 0;
    virtual void writeIntervalReboot(uint8_t interval) = 0;
    virtual int8_t readTimeZone() = 0;
    virtual void writeTimeZone(int8_t tz) = 0;
    virtual int16_t readVoltageAdj() = 0;
    virtual void writeVoltageAdj(uint16_t adj) = 0;
    virtual String readOTADriveAPIKey() = 0;
    virtual void writeOTADriveAPIKey(const String& key) = 0;
    virtual String readOTADriveURL() = 0;
    virtual void writeOTADriveURL(const String& url) = 0;
    virtual uint8_t readOTAFS() = 0;
    virtual void writeOTAFS(uint8_t fs) = 0;
    virtual String readSSID() = 0;
    virtual void writeSSID(const String& ssid) = 0;
    virtual String readPASS() = 0;
    virtual void writePASS(const String& pass) = 0;
    virtual bool EpromStatus() = 0;
    
};


#endif