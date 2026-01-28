#ifndef WIFI_SCANNER_H
#define WIFI_SCANNER_H

#include "config.h"
#include <esp_wifi.h>
#include <nvs_flash.h>

/* ========== Wi-Fi Data ========== */
// Live Monitor
extern volatile uint32_t pktTotal, pktBeacon, pktData, pktDeauth;
extern volatile int32_t rssiAccum;
extern volatile uint32_t rssiCount;
extern uint32_t history[HISTORY_SIZE];
extern uint8_t histIdx;
extern uint32_t pps, peak, lastPkt;
extern float smoothPps;
extern float avgRssi;
extern bool frozen;
extern uint8_t currentChannel;
extern uint32_t lastSecond;
extern bool signalAlert;

// Analyzer
extern uint32_t chPackets[MAX_CHANNEL + 1];
extern uint32_t chBeacons[MAX_CHANNEL + 1];
extern uint32_t chDeauth[MAX_CHANNEL + 1];
extern uint8_t analyzerChannel;
extern uint8_t selectedChannel;
extern uint32_t analyzerLastHop;

// AP Scanner
extern wifi_ap_record_t apList[MAX_APS];
extern uint16_t apCount;
extern uint8_t apCursor;
extern uint8_t apScroll;
extern uint8_t apSelectedIndex;
extern uint8_t apCompareA;
extern uint8_t apCompareB;
extern uint32_t lastScan;
extern bool apSortedOnce;

// Hidden SSIDs
extern HiddenSSID hiddenList[MAX_HIDDEN_SSIDS];
extern uint8_t hiddenCount;
extern uint8_t hiddenCursor;
extern uint8_t hiddenScroll;

// Security Counters
extern uint8_t secOpen, secWEP, secWPA, secWPA2, secWPA3;

// Session Stats
extern uint32_t sessionStart;
extern uint32_t totalAPsFound;
extern uint32_t peakPPS;
extern uint32_t totalPackets;

// Logger
extern uint32_t loggedPackets;
extern bool loggingActive;

// Deauth Detection
extern uint32_t totalDeauthDetected;
extern uint32_t deauthPerSecond;
extern uint32_t lastDeauthCount;
extern uint32_t lastDeauthCheck;
extern bool attackActive;
extern uint8_t deauthChannel;

/* ========== Wi-Fi Functions ========== */
void initWiFi();
void stopAllWifi();
void enterSnifferMode(uint8_t ch);
void enterScanMode();
void IRAM_ATTR sniffer(void* buf, wifi_promiscuous_pkt_type_t type);

void resetLiveStats();
void resetAnalyzer();
void resetSession();

void startApScan();
void sortApsByRssi();
void fetchApResults(bool forceSort = false);

uint8_t liveLoad();
const char* channelInsight();
uint8_t channelLoad(uint8_t ch);
const char* loadQuality(uint8_t load);
uint8_t bestChannel();
uint8_t bestAPIndex();

#endif // WIFI_SCANNER_H
