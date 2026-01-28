#ifndef UTILS_H
#define UTILS_H

#include "config.h"
#include <esp_wifi.h>

/* ========== MAC Vendor Database ========== */
extern const Vendor vendors[];
extern const uint8_t vendorCount;

/* ========== Helper Functions ========== */
const char* getVendor(uint8_t* mac);
float estimateDistance(int rssi);
char getQualityGrade(wifi_ap_record_t* ap);
bool hasOverlap(uint8_t ch1, uint8_t ch2);
uint8_t countOverlappingAPs(uint8_t channel);
const char* authStr(wifi_auth_mode_t m);

/* ========== External Data Access ========== */
extern wifi_ap_record_t apList[];
extern uint16_t apCount;

/* ========== External Channel Functions ========== */
uint8_t channelLoad(uint8_t ch);

#endif // UTILS_H
