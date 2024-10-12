#ifndef MAIN_H
#define MAIN_H
#include "Arduino.h"

void connectToOTAserver();
void DisconnectOTAserver();
int getFWsizeFromHeader();
void performOTA(uint32_t address);
void erasePartition(uint32_t address);
void modemConnect();
bool checkUpdate();
uint32_t writeBinaryToPartition(const String &Body, uint32_t address, size_t dataSize);
uint32_t GetSector(uint32_t Address);
void RunApplication();
void jumpToApplication(uint32_t address);
bool isAppValid(uint32_t address);
#endif