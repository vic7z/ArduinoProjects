#include "wifi_scanner.h"
#include "security.h"
#include "utils.h"

/* ========== Wi-Fi Data Variables ========== */
// Live Monitor
volatile uint32_t pktTotal = 0, pktBeacon = 0, pktData = 0, pktDeauth = 0;
volatile int32_t rssiAccum = 0;
volatile uint32_t rssiCount = 0;
uint32_t history[HISTORY_SIZE];
uint8_t histIdx = 0;
uint32_t pps = 0, peak = 0, lastPkt = 0;
float smoothPps = 0;
float avgRssi = -80;
bool frozen = false;
uint8_t currentChannel = 1;
uint32_t lastSecond = 0;

// Analyzer
uint32_t chPackets[MAX_CHANNEL + 1];
uint32_t chBeacons[MAX_CHANNEL + 1];
uint32_t chDeauth[MAX_CHANNEL + 1];
uint8_t analyzerChannel = 1;
uint8_t selectedChannel = 1;
uint32_t analyzerLastHop = 0;

// AP Scanner
wifi_ap_record_t apList[MAX_APS];
uint16_t apCount = 0;
uint8_t apCursor = 0;
uint8_t apScroll = 0;
uint8_t apSelectedIndex = 0;
uint8_t apCompareA = 0;
uint8_t apCompareB = 1;
uint32_t lastScan = 0;
bool apSortedOnce = false;

// Hidden SSIDs
HiddenSSID hiddenList[MAX_HIDDEN_SSIDS];
uint8_t hiddenCount = 0;
uint8_t hiddenCursor = 0;
uint8_t hiddenScroll = 0;

// Security Counters
uint8_t secOpen = 0, secWEP = 0, secWPA = 0, secWPA2 = 0, secWPA3 = 0;

// Session Stats
uint32_t sessionStart = 0;
uint32_t totalAPsFound = 0;
uint32_t peakPPS = 0;
uint32_t totalPackets = 0;

// Logger
uint32_t loggedPackets = 0;
bool loggingActive = false;

// Deauth Detection
uint32_t totalDeauthDetected = 0;
uint32_t deauthPerSecond = 0;
uint32_t lastDeauthCount = 0;
uint32_t lastDeauthCheck = 0;
bool attackActive = false;
uint8_t deauthChannel = 0;

/* ========== Wi-Fi Initialization ========== */
void initWiFi() {
  nvs_flash_init();
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
}

/* ========== Wi-Fi Mode Control ========== */
void stopAllWifi() {
  esp_wifi_scan_stop();
  esp_wifi_set_promiscuous(false);
}

void enterSnifferMode(uint8_t ch) {
  stopAllWifi();
  esp_wifi_set_mode(WIFI_MODE_NULL);
  esp_wifi_start();
  esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous_rx_cb(sniffer);
  esp_wifi_set_promiscuous(true);
}

void enterScanMode() {
  stopAllWifi();
  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_start();
}

/* ========== Reset Functions ========== */
void resetLiveStats() {
  pktTotal = pktBeacon = pktData = pktDeauth = 0;
  rssiAccum = rssiCount = 0;
  pps = peak = lastPkt = 0;
  smoothPps = 0;
  memset(history, 0, sizeof(history));
  histIdx = 0;
  lastSecond = millis();
}

void resetAnalyzer() {
  memset(chPackets, 0, sizeof(chPackets));
  memset(chBeacons, 0, sizeof(chBeacons));
  memset(chDeauth, 0, sizeof(chDeauth));
  analyzerChannel = 1;
  analyzerLastHop = millis();
}

void resetSession() {
  sessionStart = millis();
  totalAPsFound = 0;
  peakPPS = 0;
  totalPackets = 0;
  totalDeauthDetected = 0;
  loggedPackets = 0;
}

/* ========== AP Scan Functions ========== */
void startApScan() {
  wifi_scan_config_t cfg = {};
  cfg.show_hidden = true;
  esp_wifi_scan_start(&cfg, false);
}

void sortApsByRssi() {
  for (int i = 0; i < apCount - 1; i++) {
    for (int j = 0; j < apCount - i - 1; j++) {
      if (apList[j].rssi < apList[j + 1].rssi) {
        wifi_ap_record_t tmp = apList[j];
        apList[j] = apList[j + 1];
        apList[j + 1] = tmp;
      }
    }
  }
}

void fetchApResults(bool forceSort) {
  esp_wifi_scan_get_ap_num(&apCount);
  apCount = min(apCount, (uint16_t)MAX_APS);
  esp_wifi_scan_get_ap_records(&apCount, apList);

  totalAPsFound = max(totalAPsFound, (uint32_t)apCount);

  secOpen = secWEP = secWPA = secWPA2 = secWPA3 = 0;
  for (int i = 0; i < apCount; i++) {
    switch (apList[i].authmode) {
      case WIFI_AUTH_OPEN: secOpen++; break;
      case WIFI_AUTH_WEP: secWEP++; break;
      case WIFI_AUTH_WPA_PSK: secWPA++; break;
      case WIFI_AUTH_WPA2_PSK:
      case WIFI_AUTH_WPA_WPA2_PSK: secWPA2++; break;
      case WIFI_AUTH_WPA3_PSK: secWPA3++; break;
    }
  }

  detectRogueAPs();

  if (!apSortedOnce || forceSort) {
    sortApsByRssi();
    apCursor = 0;
    apScroll = 0;
    apSortedOnce = true;
  }
}

/* ========== Metrics ========== */
uint8_t liveLoad() {
  uint32_t score = pps + pktBeacon / 2 + pktDeauth * 3;
  return min(score / 5, 100UL);
}

const char* channelInsight() {
  // Priority 1: Security threats
  if (attackActive) return "ATTACK DETECTED!";
  if (deauthPerSecond > 5) return "Deauth Storm!";
  if (pktDeauth > pktBeacon / 4 && pktDeauth > 5) return "Possible Attack";

  // Priority 2: Load and performance
  uint8_t load = liveLoad();
  if (load > 80) return "Overloaded";
  if (load > 60) return "Congested";

  // Priority 3: Traffic patterns
  if (pktData > pktBeacon * 3) return "Heavy Traffic";
  if (pktData > pktBeacon * 2) return "Active Data";
  if (pktBeacon > pktData * 4) return "Idle/Beacons";

  // Priority 4: Quality assessment
  if (pps < 10) return "Very Clean";
  if (pps < 30) return "Clean";
  if (pps < 100) return "Moderate";

  return "Mixed Traffic";
}

uint8_t channelLoad(uint8_t ch) {
  uint32_t score = chPackets[ch] + chBeacons[ch] / 2 + chDeauth[ch] * 3;
  return min(score / 5, 100UL);
}

const char* loadQuality(uint8_t load) {
  if (load < 20) return "GOOD";
  if (load < 40) return "OK";
  if (load < 70) return "BUSY";
  return "AVOID";
}

uint8_t bestChannel() {
  uint8_t best = 1;
  uint8_t bestScore = 255;
  for (int ch = 1; ch <= MAX_CHANNEL; ch++) {
    uint8_t s = channelLoad(ch);
    if (s < bestScore && chPackets[ch] > 0) {
      bestScore = s;
      best = ch;
    }
  }
  return best;
}

uint8_t bestAPIndex() {
  uint8_t bestIdx = 0;
  char bestGrade = 'F';
  for (int i = 0; i < apCount; i++) {
    char grade = getQualityGrade(&apList[i]);
    if (grade < bestGrade || (grade == bestGrade && apList[i].rssi > apList[bestIdx].rssi)) {
      bestGrade = grade;
      bestIdx = i;
    }
  }
  return bestIdx;
}

/* ========== Promiscuous Mode Callback ========== */
void IRAM_ATTR sniffer(void* buf, wifi_promiscuous_pkt_type_t type) {
  const wifi_promiscuous_pkt_t* p = (wifi_promiscuous_pkt_t*)buf;

  pktTotal++;
  totalPackets++;
  rssiAccum += p->rx_ctrl.rssi;
  rssiCount++;

  if (type == WIFI_PKT_MGMT) {
    uint8_t st = 0;
    if (p->payload) st = (p->payload[0] >> 4) & 0x0F;

    if (st == 0x08) {
      pktBeacon++;
    } else if (st == 0x0C || st == 0x0A) {
      pktDeauth++;
      totalDeauthDetected++;
      deauthChannel = p->rx_ctrl.channel;
    } else if (st == 0x04) {
      if (p->payload && p->rx_ctrl.sig_len > 26) {
        uint8_t ssidLen = p->payload[25];
        if (ssidLen > 0 && ssidLen <= 32) {
          char probedSSID[33] = {0};
          uint8_t copyLen = (ssidLen < 32) ? ssidLen : 32;
          memcpy(probedSSID, &p->payload[26], copyLen);
          probedSSID[copyLen] = 0;

          bool found = false;
          for (int i = 0; i < hiddenCount; i++) {
            if (strcmp(hiddenList[i].ssid, probedSSID) == 0) {
              if (p->rx_ctrl.rssi > hiddenList[i].rssi) {
                hiddenList[i].rssi = p->rx_ctrl.rssi;
              }
              hiddenList[i].active = true;
              found = true;
              break;
            }
          }
          if (!found && hiddenCount < MAX_HIDDEN_SSIDS && strlen(probedSSID) > 0) {
            strcpy(hiddenList[hiddenCount].ssid, probedSSID);
            hiddenList[hiddenCount].rssi = p->rx_ctrl.rssi;
            hiddenList[hiddenCount].channel = p->rx_ctrl.channel;
            hiddenList[hiddenCount].active = true;
            hiddenCount++;
          }
        }
      }
    }
  } else if (type == WIFI_PKT_DATA) {
    pktData++;
  }

  // CSV Logging
  if (loggingActive && settings.csvLogging && loggedPackets < 1000) {
    loggedPackets++;
    // Format: timestamp,channel,rssi,type
    Serial.printf("%lu,%d,%d,%d\n", millis(), p->rx_ctrl.channel,
                  p->rx_ctrl.rssi, type);
  }
}
