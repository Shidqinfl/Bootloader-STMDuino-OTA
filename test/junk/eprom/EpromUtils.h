#ifndef EPROMUTILS_H
#define EPROMUTILS_H
#include "Configurator.h"
#include "IEpromUtils.h"
class EpromUtils
{
private:
    /* data */
public:
    EpromUtils(IEprom &ieprom):_ieprom(ieprom){}
        void begin(){
            _ieprom.begin();
        }
        void clearAll(){
            _ieprom.clearAll();
        }
        bool validate(){
            return _ieprom.validate();
        }
        void writeBrokerStr(const std::string bconf[4]){
            _ieprom.writeBrokerStr(bconf);
        }
        void readBrokerStr(){
            _ieprom.readBrokerStr();
        }
        uint8_t readByte(uint16_t address){
            return _ieprom.readByte(address);
        }
        void writeByte(uint16_t address, uint8_t data){
            _ieprom.writeByte(address, data);
        }
        String readModelAC(){
            return readModelAC();
        }
        void writeModelAC(const String& model){
            writeModelAC(model);
        }
        uint8_t readIntervalSend(){
            return readIntervalSend();
        }
        void writeIntervalSend(uint8_t interval){
            writeIntervalSend(interval);
        }
        uint8_t readIntervalReboot(){
            readIntervalReboot();
        }
        void writeIntervalReboot(uint8_t interval);
        int8_t readTimeZone();
        void writeTimeZone(int8_t tz);
        int16_t readVoltageAdj();
        void writeVoltageAdj(uint16_t adj);
        String readOTADriveAPIKey();
        void writeOTADriveAPIKey(const String& key);
        String readOTADriveURL();
        void writeOTADriveURL(const String& url);
        uint8_t readOTAFS();
        void writeOTAFS(uint8_t fs);
        String readSSID();
        void writeSSID(const String& ssid);
        String readPASS();
        void writePASS(const String& pass);
        bool EpromStatus();
private:
    IEprom &_ieprom;
};



#endif