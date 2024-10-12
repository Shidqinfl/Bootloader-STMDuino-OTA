#include "GSMComs.h"
#include "configurator.h"
#include <ctime>

/*  Preferred mode selection : AT+CNMP
        2 – Automatic
        13 – GSM Only
        14 – WCDMA Only
        38 – LTE Only
        59 – TDS-CDMA Only
        9 – CDMA Only
        10 – EVDO Only
        19 – GSM+WCDMA Only
        22 – CDMA+EVDO Only
        48 – Any but LTE
        60 – GSM+TDSCDMA Only
        63 – GSM+WCDMA+TDSCDMA Only
        67 – CDMA+EVDO+GSM+WCDMA+TDSCDMA Only
        39 – GSM+WCDMA+LTE Only
        51 – GSM+LTE Only
        54 – WCDMA+LTE Only
*/

static volatile int ringCount = 0;

void __attribute__((section(".data"))) onRing(){
    ringCount++;
}

GSMComs::GSMComs(TinyGsm &modem, Logger &logger) : _modem(modem), _logger(logger)
{
    _net_fail = true;
    _eventCallback = nullptr;
    _ringCountLimit = GSM_RING_COUNT_LIMIT;
}

bool GSMComs::resetModem()
{
    _modem.sendAT("AT+CRESET");
    _modem.waitResponse(GF(GSM_NL));
    String onoff = _modem.stream.readStringUntil('\n');
    _logger.log(LogLevel::INFO, "reset modem: " + onoff);
    if (strstr(onoff.c_str(), "OK")!= NULL)
    {
        return true;
    }
    return false;
}

void GSMComs::init(uint8_t mode_modem)
{
    _logger.log(LogLevel::INFO, "LTE Initialize");
    
    // _modem.begin();
    pinMode(MODEM_PWRKEY, OUTPUT);
    pinMode(MODEM_DCD, INPUT);
    pinMode(MODEM_STATUS , INPUT);

    for (size_t i = 0; i < 10; i++)
    {
        digitalWrite(MODEM_PWRKEY, LOW);
        // _logger.log(LogLevel::INFO, "LOW");
        delay(300);
        digitalWrite(MODEM_PWRKEY, HIGH);
        // _logger.log(LogLevel::INFO, "HIGH");
        delay(300);
        digitalWrite(MODEM_PWRKEY, LOW);
        // delay(12000);
        // _logger.log(LogLevel::INFO, "MODEM_STATUS for: "+ String(digitalRead(MODEM_STATUS)));
        if(digitalRead(MODEM_STATUS)==1){
            i = 10;
            
        }
        _logger.log(LogLevel::INFO, "MODEM_STATUS "+ String(digitalRead(MODEM_STATUS)));
        
    }
    _logger.log(LogLevel::INFO, "MODEM_DCD   : "+ String(digitalRead(MODEM_DCD))); 


    // TinyGsmAutoBaud(MODEM, 9600, 115200);
    pinMode(MODEM_RI, INPUT_PULLDOWN);
    attachInterrupt(digitalPinToInterrupt(MODEM_RI), onRing, RISING);
    
    _logger.log(LogLevel::INFO, "Initializing modem...");
    if (!_modem.init())
    {
        _logger.log(LogLevel::ERROR, "Failed to initialize modem, retry in 10s");
    }

    // Modem Config
    String ret;
    ret = _modem.setNetworkMode(mode_modem); // at+CNMP
    _logger.log(LogLevel::DEBUG, "setNetworkMode: " + ret);

    String name = _modem.getModemName();
    _logger.log(LogLevel::DEBUG, "Modem Name: " + name);

    String modemInfo = _modem.getModemInfo();
    _logger.log(LogLevel::DEBUG, "Modem Info: " + modemInfo);
    
    // _modem.sendAT("AT+CPIN?");
    _modem.sendAT(GF("+CPIN?"));
    _modem.waitResponse(GF(GSM_NL));
    String pin = _modem.stream.readStringUntil('\n');
    _modem.waitResponse();
    _logger.log(LogLevel::DEBUG, "ATCPIN : " + pin);

    // _modem.sendAT(GF("+IPR?"));
    // _modem.waitResponse(GF(GSM_NL));
    // String speed = _modem.stream.readStringUntil('\n');
    // _modem.waitResponse();
    // _logger.log(LogLevel::DEBUG, "ATIPR : " + speed);

    // _modem.sendAT(GF("+IPR=115200"));
    // _modem.waitResponse(GF(GSM_NL));
    // speed = _modem.stream.readStringUntil('\n');
    // _modem.waitResponse();
    // _logger.log(LogLevel::DEBUG, "ATIPR : " + speed);



#if TINY_GSM_USE_GPRS
    // Unlock your SIM card with a PIN if needed
    if (GSM_PIN && _modem.getSimStatus() != 3)
    {
        _modem.simUnlock(GSM_PIN);
    }
#endif

#if TINY_GSM_USE_WIFI
    // Wifi connection parameters must be set before waiting for the network
    _logger.log(LogLevel::DEBUG, "Setting SSID/password...");
    if (!_modem.networkConnect(wifiSSID, wifiPass))
    {
        _logger.log(LogLevel::ERROR, "Failed to run wifi");
        delay(10000);
        return;
    }
    _logger.log(LogLevel::DEBUG, "Success to run wifi");
#endif

    _logger.log(LogLevel::DEBUG, "Waiting for network...");
    if (!_modem.waitForNetwork())
    {
        _logger.log(LogLevel::ERROR, "Failed to connect to network");
        _net_fail = true;
        delay(5000);
        return;
    }

    if (_modem.isNetworkConnected())
    {
        _logger.log(LogLevel::DEBUG, "Network connected");
        _net_fail = false;
        _modem.waitResponse(GF(GSM_NL));
    }

#if TINY_GSM_USE_GPRS
    // GPRS connection parameters are usually set after network registration
    _logger.log(LogLevel::DEBUG, "Connecting to " + String(apn));
    if (!_modem.gprsConnect(apn, gprsUser, gprsPass))
    {
        _logger.log(LogLevel::ERROR, "Failed to connect to " + String(apn));
        _net_fail = true;
        delay(5000);
        return;
    }
    if (_modem.isGprsConnected())
    {
        _logger.log(LogLevel::DEBUG, "GPRS connected");
        _net_fail = false;
    }
#endif
}

bool GSMComs::loop()
{
    // Check ring count
    if (ringCount >= _ringCountLimit)
    {
        _logger.log(LogLevel::DEBUG, "Ring count limit reached");
        if (_eventCallback != nullptr)
        {
            _eventCallback(COM_EVENT_RING_LIMIT_REACHED);
        }

        // Reset ring count
        ringCount = 0;
    }

    // Check network status
    if (!_modem.isNetworkConnected())
    {
        _logger.log(LogLevel::ERROR, "Network disconnected");
        _net_fail = true;
        if (_eventCallback != nullptr)
        {
            _eventCallback(COM_EVENT_NET_FAIL);
        }
        if (!_modem.waitForNetwork(MODEM_TIMEOUT_MS, true))
        {
            _logger.log(LogLevel::ERROR, "Network fail");
            _net_fail = true;
            if (_eventCallback != nullptr)
            {
                _eventCallback(COM_EVENT_NET_FAIL);
            }
            delay(MODEM_DELAY_MS);
            return false;
        }
        if (_modem.isNetworkConnected())
        {
            _logger.log(LogLevel::DEBUG, "Network reconnect");
            _net_fail = false;
            if (_eventCallback != nullptr)
            {
                _eventCallback(COM_EVENT_NET_OK);
            }
        }
#if TINY_GSM_USE_GPRS
        // and make sure GPRS/EPS is still connected
        if (!_modem.isGprsConnected())
        {
            _logger.log(LogLevel::ERROR, "GPRS not connected");
            _logger.log(LogLevel::DEBUG, "Connecting to " + String(apn));
            if (!_modem.gprsConnect(apn, gprsUser, gprsPass))
            {
                _logger.log(LogLevel::ERROR, "GPRS connect fail");
                if (_eventCallback != nullptr)
                {
                    _eventCallback(COM_EVENT_NET_FAIL);
                }

                delay(MODEM_DELAY_MS);
                return false;
            }
            if (_modem.isGprsConnected())
            {
                _logger.log(LogLevel::DEBUG, "GPRS reconnect");
                _net_fail = false;
                if (_eventCallback != nullptr)
                {
                    _eventCallback(COM_EVENT_NET_OK);
                }
            }
        }
#endif
    }

    _modem.maintain();

    return true;
}

void GSMComs::setEventCallback(EventCallback callback)
{
    _eventCallback = callback;
}

bool GSMComs::checkNetFail()
{
    return _net_fail;
}

int GSMComs::checkQuality()
{
    return _modem.getSignalQuality();
}

bool GSMComs::checktime(char* timecharArr)
{
    int iYear, iMonth, iDay, iHour, iMin, iSec = 0;
    float fTz = 0;
    bool ct = _modem.getNetworkTime(&iYear, &iMonth, &iDay, &iHour, &iMin, &iSec, &fTz);

    if (!ct)
    {
        Serial.println("[TIME][ERROR] invalid format");
        return false;
    }

    int tz_h = (int)fTz;               // get the hour part
    int tz_m = abs((fTz - tz_h) * 60); // get the minute part
    char tzStr[10];
    memset(tzStr, 0, sizeof(tzStr));
    sprintf(tzStr, "%s%02d:%02d", fTz >= 0 ? "+" : "-", abs(tz_h), tz_m);

    // char t[200];
    // memset(t, 0, sizeof(t));
    sprintf(timecharArr, "%s-%s-%sT%s:%s:%s%s", String(iYear).c_str(), PAD_WITH_ZERO(iMonth).c_str(), PAD_WITH_ZERO(iDay).c_str(), PAD_WITH_ZERO(iHour).c_str(), PAD_WITH_ZERO(iMin).c_str(),PAD_WITH_ZERO(iSec).c_str(), String(tzStr).c_str());
    // timestr = String(t);

    return true;
}

// Ring Handler
int GSMComs::getRingCount()
{
    return ringCount;
}

void GSMComs::setRingCountLimit(int count)
{
    _ringCountLimit = count;
}

void GSMComs::resetRingCount()
{
    ringCount = 0;
}

// USSD
String GSMComs::sendUSSD(String ussd)
{
    resetRingCount(); // prevent from restarting
    _modem.sendAT(GF("+CMGF=1"));
    _modem.waitResponse();

    // Set preferred message format to text mode
    _modem.sendAT(GF("+CSCS=\"IRA\""));
    _modem.waitResponse();

    // Send the message
    _modem.sendAT(GF("+CUSD=1,\""), ussd, GF("\",15"));
    if (_modem.waitResponse() != 1)
    {
        return "";
    }
    if (_modem.waitResponse(30000L, GF("+CUSD:")) != 1)
    {
        return "";
    }

    // Read response
    _modem.stream.readStringUntil('"');
    String str = _modem.stream.readStringUntil('"');

    return str;
}

void GSMComs::cancelUSSD()
{
    _modem.sendAT(GF("+CUSD=2"));
    _modem.waitResponse(30000L);
}

// SMS
bool GSMComs::sendSMS(String number, String message)
{
    return _modem.sendSMS(number, message);
}

// Read SMS
/* Sample message

    +CMGL: 6,"REC UNREAD","846976757977836976","","24/06/04,12:00:28+28"<\r><\n>
    Anda memiliki<\r><\n>
    Kuota Internet 1023.86 MB berlaku s.d. 30-06-2024 pkl 23:59<\r><\n>
    <\r><\n>
    <\r><\n>
    Kuota-mu masih banyak? Yuk Transfer Kuota ke teman-temanmu di  *500*2#. (Hany<\r><\n>
    +CMGL: 7,"REC UNREAD","846976757977836976","","24/06/04,12:00:28+28"<\r><\n>
    a utk PraBayar). Info detail kuota lokal, klik tsel.me/loka12:19:39+28"<\r><\n>
    2<\r><\n>
    +CMGL: 19,"REC UNREAD","+6282215481250","","24/06/04,12:19:55+28"<\r><\n>
    3<\r><\n>
    +CMGL: 20,"REC UNREAD","84697675711:19:37+28"<\r><\n>
    a utk PraBayar). Info detail kuota lokal, klik tsel.me/lokal<\r><\n>
    <\r><\n>

*/
// TODO: Fix Bad heap when reading SMS
int GSMComs::readSMS(SMS_t *sms, int index, String searchStr)
{
    String response;
    int prevSmsId = -1;
    SMS_t curSms;
    int smsCountIdx = 0;

    enum readSMSState
    {
        READ_IDLE,
        READ_CMGL,
        READ_NEW_MSG,
        READ_CONT_MSG
    } state = READ_IDLE;

    // Check if sms and smsCount var not empty
    if (sms == nullptr)
    {
        _logger.log(LogLevel::ERROR, "SMS array is empty");
        return -1;
    }

    // Get size of SMS array
    int smsSize = sizeof(sms) / sizeof(SMS_t);

    memset(&curSms, 0, sizeof(SMS_t));

    // If index == -1 then read latest sms, if not use index
    if (index == -1)
    {
        _modem.sendAT(GF("+CMGF=1"));
        _modem.waitResponse();
        _modem.sendAT(GF("+CMGL=\"REC UNREAD\""));
        _modem.waitResponse(GF(GSM_NL));

        do
        {
            response = _modem.stream.readStringUntil('\n');
            if (response.startsWith("+CMGL:"))
            {
                // Finished reading SMS
                if (state == READ_CONT_MSG)
                {
                    // Check if we focus only on specific message
                    if (searchStr != "")
                    {
                        String rspLower = response;
                        rspLower.toLowerCase();
                        searchStr.toLowerCase();
                        if (rspLower.indexOf(searchStr) != -1)
                        {
                            _logger.log(LogLevel::DEBUG, "SMS Found: " + curSms.message);
                            _logger.log(LogLevel::DEBUG, "Sender: " + curSms.sender + ", Date: " + curSms.date);

                            memcpy(&sms[smsCountIdx++], &curSms, sizeof(SMS_t));
                        }
                    }
                    else
                    {
                        memcpy(&sms[smsCountIdx++], &curSms, sizeof(SMS_t));
                        _logger.log(LogLevel::DEBUG, "Sender: " + curSms.sender + ", Date: " + curSms.date + ", Message: " + curSms.message);
                    }

                    // Finished reading SMS or full
                    if (smsCountIdx >= smsSize)
                    {
                        return smsSize;
                    }
                }

                state = READ_CMGL;

                // +CMGL: 6,"REC UNREAD","846976757977836976","","24/06/04,12:00:28+28"<\r><\n> --> meta sample
                String cmgl = response.substring(7);

                // Split the response into an array of strings
                int maxFields = 6;
                String fields[maxFields];
                int fieldIndex = 0;

                // Parse each field
                int start = 0;

                int end = cmgl.indexOf(',');
                while (end != -1 && fieldIndex < maxFields)
                {
                    fields[fieldIndex] = cmgl.substring(start, end);
                    start = end + 1;
                    end = cmgl.indexOf(',', start);
                    fieldIndex++;
                }

                // Get the last field
                if (fieldIndex < maxFields)
                {
                    fields[fieldIndex] = cmgl.substring(start);
                }

                // Initiate with first message
                int curSmsId = fields[0].toInt();
                String curSmsSender = fields[2];

                if (prevSmsId == -1)
                {
                    // Initiate with first message / new message
                    state = READ_NEW_MSG;
                }
                else if (curSmsId - prevSmsId == 1 && curSmsSender == curSms.sender)
                {
                    // Consecutive mesasge
                    state = READ_CONT_MSG;
                }
                else
                {
                    // New message
                    state = READ_NEW_MSG;
                }

                curSms.sender = fields[2];
                curSms.date = fields[4];
                prevSmsId = curSmsId;
            }
            else
            {
                switch (state)
                {
                case READ_NEW_MSG:
                    curSms.message = response;
                    break;
                case READ_CONT_MSG:
                    curSms.message += response;
                    break;
                default:
                    break;
                }
            }
        } while (response.length() > 0);

        // No more message, finish this
        if (state == READ_CONT_MSG)
        {
            // Check if we focus only on specific message
            if (searchStr != "")
            {
                String rspLower = response;
                rspLower.toLowerCase();
                searchStr.toLowerCase();
                if (rspLower.indexOf(searchStr) != -1)
                {
                    _logger.log(LogLevel::DEBUG, "SMS Found: " + curSms.message);
                    _logger.log(LogLevel::DEBUG, "Sender: " + curSms.sender + ", Date: " + curSms.date);

                    memcpy(&sms[smsCountIdx++], &curSms, sizeof(SMS_t));
                }
            }
            else
            {
                memcpy(&sms[smsCountIdx++], &curSms, sizeof(SMS_t));
                _logger.log(LogLevel::DEBUG, "Sender: " + curSms.sender + ", Date: " + curSms.date + ", Message: " + curSms.message);
            }
        }
    }
    else
    {
        _modem.sendAT(GF("+CMGF=1"));
        _modem.waitResponse();
        _modem.sendAT(GF("+CMGR="), index);
        _modem.waitResponse(GF(GSM_NL));

        response = _modem.stream.readStringUntil('\n');
        if (response.startsWith("+CMGR:"))
        {
            // +CMGR: "REC READ","846976757977836976","","24/06/04,11:53:06+28"<\r><\n>
            String cmgr = response.substring(7);

            // Split the response into an array of strings
            int maxFields = 6;
            String fields[maxFields];
            int fieldIndex = 0;

            // Parse each field
            int start = 0;
            int end = cmgr.indexOf(',');
            while (end != -1 && fieldIndex < maxFields)
            {
                fields[fieldIndex] = cmgr.substring(start, end);
                start = end + 1;
                end = cmgr.indexOf(',', start);
                fieldIndex++;
            }

            // Get the last field
            if (fieldIndex < maxFields)
            {
                fields[fieldIndex] = cmgr.substring(start);
            }

            // Sender
            curSms.sender = fields[2];

            // Date
            curSms.date = fields[4];

            // Message
            response = _modem.stream.readStringUntil('\n');
            response.trim();
            if (response.length() > 0)
            {
                curSms.message = response;

                // Check if we focus only on specific message
                if (searchStr != "")
                {
                    String rspLower = response;
                    rspLower.toLowerCase();
                    searchStr.toLowerCase();
                    if (rspLower.indexOf(searchStr) != -1)
                    {
                        _logger.log(LogLevel::DEBUG, "SMS Found: " + curSms.message);
                        _logger.log(LogLevel::DEBUG, "Sender: " + curSms.sender + ", Date: " + curSms.date);

                        memcpy(&sms[smsCountIdx++], &curSms, sizeof(SMS_t));
                    }
                }
                else
                {
                    memcpy(&sms[smsCountIdx++], &curSms, sizeof(SMS_t));
                    _logger.log(LogLevel::DEBUG, "Sender: " + curSms.sender + ", Date: " + curSms.date + ", Message: " + curSms.message);
                }
            }
        }
    }

    return smsCountIdx;
}

void GSMComs::clearSMS()
{
    _modem.sendAT(GF("+CMGD=1,4"));
    _modem.waitResponse();
}

// Phone Number
String GSMComs::getPhoneNumberFromSIM()
{
    _modem.sendAT(GF("+CNUM"));
    _modem.waitResponse(GF(GSM_NL));
    String cnum = _modem.stream.readStringUntil('\n');
    _modem.waitResponse();

    int firstQuote = cnum.indexOf('\"');
    int lastQuote = cnum.lastIndexOf('\"');

    if (firstQuote == -1 || lastQuote == -1 || lastQuote <= firstQuote)
    {
        return "";
    }

    String number = cnum.substring(firstQuote + 1, lastQuote);
    number.trim();

    return number;
}

// Get Phone Number from USSD
String GSMComs::getPhoneNumberFromUSSD()
{
    // TODO Fix USSD Handler
    String res = "";
    String phoneNum = "";

    cancelUSSD(); // Close connection first if any
    sendUSSD("*808#");
    _logger.log(LogLevel::DEBUG, "USSD: *808#");
    res = sendUSSD("1");

    if (res.length() > 0)
    {
        int startPos = res.indexOf("62");
        int endPos = res.indexOf(".");
        if (startPos != -1 && endPos != -1 && endPos > startPos)
        {
            phoneNum = res.substring(startPos, endPos);
        }
    }

    cancelUSSD(); // Close connection

    return phoneNum;
}

// Balance
String GSMComs::getBalance()
{
    cancelUSSD(); // Close connection first if any
    String str = sendUSSD("*888#");
    cancelUSSD(); // Close connection

    // Get text between "Rp " and "."
    String balance = str.substring(str.indexOf("Rp ") + 3, str.indexOf("."));

    return balance;
}

// Quota
String GSMComs::getQuota()
{
    cancelUSSD(); // Close connection first if any
    String str = sendUSSD("*888*3#");
    cancelUSSD(); // Close connection

    _logger.log(LogLevel::DEBUG, "Quota USSD: " + str);

    // Wait 5 s
    delay(5000);

    // Read latest SMS
    String res = "";
    SMS_t sms[1];
    int smsCount = 0;

    memset(sms, 0, sizeof(sms));

    int rInt = readSMS(sms, -1, "internet");
    if (rInt > 0)
    {
        res = sms[0].message;
    }

    return res;
}