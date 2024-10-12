
#include "main.h"
#include "Configurator.h"
#include <TinyGsmClient.h>
#include <stm32f4xx_hal_flash.h>
#include "stm32f4xx_hal_flash_ex.h"

#define APP_VALID_FLAG 0x20020000
HardwareSerial DEBUG_SERIAL(DEBUG_TX, DEBUG_RX);
//tinyGSM
TinyGsm modem(MODEM);
TinyGsmClient client(modem, 1);
const int chunkSize = 1024;

const String FIRMWARE_path = "/firmware.bin";
const String SERVER_HOST   = "[YOUR_URL]:[YOUR_URL_PORT]"; //change with your server

uint32_t APPLICATION_1_ADDRESS = 0x08010000;
String Header;

void setup() {
  delay(5000);
  DEBUG_SERIAL.begin(115200);
  MODEM.begin(115200);
  DEBUG_SERIAL.println("");
  DEBUG_SERIAL.println("[DEBUG][BOOT] STARTING...");

  if (!isAppValid(APPLICATION_1_ADDRESS))
  {
    DEBUG_SERIAL.println("[DEBUG][BOOT] Downloading App");
    modemConnect();
    connectToOTAserver();
    performOTA(APPLICATION_1_ADDRESS);
    DisconnectOTAserver();
  }
  RunApplication();
}

void loop() {
  
}

void modemConnect(){
  DEBUG_SERIAL.println("[DEBUG][SIM] Modem Start");
  pinMode(MODEM_PWRKEY, OUTPUT);
  pinMode(MODEM_DCD, INPUT);
  pinMode(MODEM_STATUS , INPUT);
  digitalWrite(MODEM_PWRKEY, LOW);
  delay(300);
  digitalWrite(MODEM_PWRKEY, HIGH);
  // delay(300);
  // digitalWrite(MODEM_PWRKEY, LOW);
  modem.init();
  modem.setNetworkMode(2);
  modem.sendAT("AT+CHTTPSRECV=1460");
  modem.waitResponse();

  if (!modem.waitForNetwork(600000L, true)){
    DEBUG_SERIAL.println("[ERROR][SIM] waiting Netfail");
  }
  if (modem.isNetworkConnected()) {
    DEBUG_SERIAL.println("[INFO][SIM] Network connected");
  }
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    DEBUG_SERIAL.println("[ERROR][SIM] GPRS confail");
  }
  if (modem.isGprsConnected()) { DEBUG_SERIAL.println("[INFO][SIM] GPRS connected"); }
  DEBUG_SERIAL.println("[DEBUG][SIM] Modem Finish");
}

int getFWsizeFromHeader() {
  DEBUG_SERIAL.print("[DEBUG][FOTA] GET FIRMWARE SIZE\n");
  uint32_t fwsize;
  bool isHeader = true;
  String request = "HEAD "+FIRMWARE_path+" HTTP/1.1\r\n";
  request += "Host: " +SERVER_HOST+"\r\n";
  request += "Connection: keep-alive\r\n\r\n";
  client.setTimeout(1000);
  client.print(request);
  delay(100);
  while (client.connected() || client.available()) {
    if (client.available()) {
      char c = client.read();
      if (c == '\n'){
        if (Header.length()==0)
        {
          isHeader = false;
          continue;
        }
        if (Header.startsWith("Content-Length"))
        {
          fwsize = Header.substring(15).substring(Header.substring(15).indexOf(':')+1).toInt();
        }
        Header = "";
      }else if(c!= '\r'){
        Header+=c;
      }
    }else{
      break;
    }
  }
  DEBUG_SERIAL.print("[DEBUG][FOTA] FW SIZE: ");DEBUG_SERIAL.println(fwsize);
  return fwsize ;
}

void connectToOTAserver(){
  if (!client.connect("[YOUR_URL]", 0)) { //change with your server configuration
    DEBUG_SERIAL.println("[ERROR][FOTA] Connection to server failed");
    return;
  }else{
    DEBUG_SERIAL.println("[DEBUG][FOTA] Connected to server");
  }
}
void DisconnectOTAserver(){
  client.stop();
}

void performOTA( uint32_t address){
  
  int firmwareSize = getFWsizeFromHeader();
  char c;
  uint32_t FirstBytes = 0;
  uint32_t LastBytes = 1023;
  bool isHeader = true;
  String Body;
  int bufsize = LastBytes+1;
  uint8_t nLoop = (firmwareSize+bufsize-1)/bufsize;
  uint8_t j;

  if (firmwareSize!=0)
  {
    erasePartition(address);
    for (size_t i = 1; i <= nLoop; i++)
    {
      DEBUG_SERIAL.println("\n========================================");
      if (i >= nLoop)
      {
        LastBytes = firmwareSize;
        DEBUG_SERIAL.println("[DEBUG][FOTA] last FW Chunk");
      }
      String request = "GET "+FIRMWARE_path+" HTTP/1.1\r\n";
      request += "Host: " +SERVER_HOST+"\r\n";
      request += "Connection: keep-alive\r\n";
      request += "Range: bytes="+String(FirstBytes)+"-"+String(LastBytes)+"\r\n\r\n";
      client.setTimeout(1000);
      DEBUG_SERIAL.println("[DEBUG][FOTA] HTTP send request : ");
      DEBUG_SERIAL.println(request);
      client.print(request);
      modem.waitResponse();
      while (client.connected() || client.available()) {
        if (client.available()) {
          c = client.read();
          if (isHeader)
          {
            if (c == '\n')
            {
              if (Header.length()==0)
              {
                isHeader = false;
                continue;
              }
              Header = "";
            }else if(c!= '\r'){
              Header+=c;
            }
          }else{ //isi firmware ada disini
            Body+=c;
            DEBUG_SERIAL.printf(" %02X",c);
          }
        }else{
          DEBUG_SERIAL.println("\r\n no data avail ");
          delay(500);
          if (j>3){j = 0;break;}j++;
        }
        delay(1);
      }
      if(i < nLoop){
        DEBUG_SERIAL.printf("\n[DEBUG][FOTA] Progress: %d / %d \n", Body.length() * i, firmwareSize);
      }else{
        DEBUG_SERIAL.printf("\n[DEBUG][FOTA] Progress: %d / %d \n", LastBytes, firmwareSize);
      }
      
      address = writeBinaryToPartition(Body, address, Body.length());
      Body   ="";
      Body.trim();
      Header ="";
      isHeader = true;
      if (LastBytes != firmwareSize)
      {
        FirstBytes = LastBytes+1;
        LastBytes  = (LastBytes+bufsize)-1;
      }
      

      modem.waitResponse();
    }
  }
}

bool checkUpdate(){
  return 0;
}

void erasePartition(uint32_t address){
  DEBUG_SERIAL.println("[DEBUG][FOTA] Erase Partition");
  HAL_FLASH_Unlock();
  FLASH_EraseInitTypeDef EraseInitStruct;
  uint32_t SectorError = 0;
  EraseInitStruct.TypeErase    = FLASH_TYPEERASE_SECTORS;
  EraseInitStruct.Sector       = GetSector(address);
  EraseInitStruct.NbSectors    = 2;
  EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
  if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK)
  {
    DEBUG_SERIAL.println("[ERROR][FOTA] Erase sector failed or Sector already empty");
  }
  HAL_FLASH_Lock();  
}

uint32_t writeBinaryToPartition(const String &Body, uint32_t address, size_t dataSize){
  DEBUG_SERIAL.printf("[DEBUG][FOTA] Inject Firmware to Flash\n");
  DEBUG_SERIAL.printf("[DEBUG][FOTA] Size Fw to Flash:%d \n", dataSize);
  // DEBUG_SERIAL.print("[DEBUG][FOTA] Size Fw to Flash: ");DEBUG_SERIAL.println(dataSize);
  HAL_FLASH_Unlock();
  for (size_t i = 0; i < dataSize; i++)
  { 
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address + i, Body[i])!= HAL_OK)
    {
      // HAL_FLASH_Lock();
      DEBUG_SERIAL.println("[ERROR][FOTA] Write bin failed");
      return address;
    }else{

    }
  }
  HAL_FLASH_Lock();  
  DEBUG_SERIAL.printf("[DEBUG][FOTA] Finish Flash on 0x%02X - 0x%02X \n\n", address, (address+ dataSize));
  return address+dataSize;
}

uint32_t GetSector(uint32_t Address) {
  if (Address == 0x08010000){
    return FLASH_SECTOR_4;
  }else if (Address == 0x08040000){
    return FLASH_SECTOR_6;
  }
  return FLASH_SECTOR_7;
}

void RunApplication(){
  if (isAppValid(APPLICATION_1_ADDRESS)) {
    DEBUG_SERIAL.println("[DEBUG][BOOT] Jumping to APP");
    jumpToApplication(APPLICATION_1_ADDRESS);
  } else {
    DEBUG_SERIAL.println("[DEBUG][BOOT] No valid application found");
    delay(3000);
    HAL_NVIC_SystemReset();
  }
}

__attribute__((optimize("O0")))
void jumpToApplication(uint32_t address){
  typedef void (*pFunction)(void);
  pFunction JumpToApp;
  DEBUG_SERIAL.print("[DEBUG][BOOT] jump to partition : 0x"); DEBUG_SERIAL.println(address,HEX);
  delay(3000);
  SCB->VTOR = address;
  __set_MSP(*(__IO uint32_t*) address);
  JumpToApp =  (pFunction) *(__IO uint32_t*) (address + 4); 
  JumpToApp();
}

bool isAppValid(uint32_t address) {
  // Read the first word of the application
  uint32_t appStartAddress = address;
  uint32_t magicNumber = *(__IO uint32_t*) appStartAddress;
  DEBUG_SERIAL.print("[DEBUG][BOOT] cek partition : 0x"); DEBUG_SERIAL.println(address,HEX);
  DEBUG_SERIAL.print("[DEBUG][BOOT] magicnumber   : 0x"); DEBUG_SERIAL.println(magicNumber,HEX);
  if (magicNumber == APP_VALID_FLAG ) //0xFFFFFFFF
  {
    return true;
  }
  
  return false;
}