#ifndef CONFIGURATOR_H
#define CONFIGURATOR_H
#include "Arduino.h"

//LED
#define LED_PIN   PC13
//Debug Serial
#define DEBUG_RX PB6  // PA12 rts
#define DEBUG_TX PB7  // PA11 cts
#define DEBUG_SERIAL Serial6
// GSM Modem
#define TINY_GSM_MODEM_SIM7600
#define TINY_GSM_USE_WIFI false
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_HEX true
#define MODEM_TIMEOUT_MS 180000L
#define MODEM_DELAY_MS 10000
#define MODEM_UART_BAUD 115200
// Debug GSM
#define TINY_GSM_DEBUG DEBUG_SERIAL
#define MODEM Serial2
// GSM
#define GSM_PIN ""
const char apn[] = "internet";
const char gprsUser[] = "";
const char gprsPass[] = "";
#define GSM_RING_COUNT_LIMIT 3

#define MODEM_TX        PA2
#define MODEM_RX        PA3
#define MODEM_DTR       PA5
#define MODEM_RI        PA6
#define MODEM_STATUS    PA7
#define MODEM_DCD       PA4 
#define MODEM_RTS       PA0
#define MODEM_CTS       PA1
#define MODEM_PWRKEY    PB10

#if !defined(TINY_GSM_RX_BUFFER)
#define TINY_GSM_RX_BUFFER 512
#endif


#endif