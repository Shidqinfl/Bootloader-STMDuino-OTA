#ifndef COMS_H
#define COMS_H

#include <Arduino.h>
#include "configurator.h"
#include "Logger.h"
#include <TinyGSM.h>

enum ComEventType
{
    COM_EVENT_NET_FAIL,
    COM_EVENT_NET_OK,
    COM_EVENT_NET_QUALITY,
    COM_EVENT_TIME,
    COM_EVENT_DOWNLOAD,
    COM_EVENT_RING_LIMIT_REACHED
};

typedef void (*EventCallback)(enum ComEventType);

class GSMComs
{
public:
    typedef struct SMS
    {
        String sender;
        String date;
        String message;
    } SMS_t;
    GSMComs(TinyGsm &modem, Logger &logger);
    void init(uint8_t mode_modem);
    bool loop();

    void setEventCallback(EventCallback callback);
    bool checkNetFail();
    int checkQuality();
    bool checktime(char* timecharArr);

    int getRingCount();
    void setRingCountLimit(int count);
    void resetRingCount();
    bool resetModem();

    String sendUSSD(String ussd);
    void cancelUSSD();
    bool sendSMS(String number, String message);
    int readSMS(SMS_t *sms, int index = -1, String searchStr = "");
    void clearSMS();
    String getPhoneNumberFromSIM();
    String getPhoneNumberFromUSSD();
    String getQuota();
    String getBalance();

private:
    Logger &_logger;
    TinyGsm _modem;

    // Network
    int _ringCountLimit;
 
    bool _net_fail;
    EventCallback _eventCallback;
};

#endif