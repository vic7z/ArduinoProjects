#include "utils.h"
#include "wifi_scanner.h"

/* ========== MAC Vendor Database ========== */
const Vendor vendors[] = {
  {{0x00, 0x03, 0x93}, "Apple"},
  {{0x00, 0x50, 0xF2}, "Microsoft"},
  {{0x00, 0x1B, 0x63}, "Apple"},
  {{0x3C, 0x22, 0xFB}, "Apple"},
  {{0xDC, 0x2C, 0x6E}, "Apple"},
  {{0x28, 0xE1, 0x4C}, "Samsung"},
  {{0xE8, 0x50, 0x8B}, "Samsung"},
  {{0x34, 0x08, 0xBC}, "Samsung"},
  {{0xD8, 0xEB, 0x97}, "Raspberry"},
  {{0xB8, 0x27, 0xEB}, "Raspberry"},
  {{0xDC, 0xA6, 0x32}, "Raspberry"},
  {{0x18, 0xFE, 0x34}, "Espressif"},
  {{0xAC, 0x67, 0xB2}, "Espressif"},
  {{0x24, 0x0A, 0xC4}, "Espressif"},
  {{0x84, 0xCC, 0xA8}, "Tp-Link"},
  {{0xC0, 0x4A, 0x00}, "Tp-Link"},
  {{0xE8, 0x94, 0xF6}, "Tp-Link"}
};
const uint8_t vendorCount = sizeof(vendors) / sizeof(Vendor);

/* ========== MAC Vendor Lookup ========== */
const char* getVendor(uint8_t* mac) {
  for (int i = 0; i < vendorCount; i++) {
    if (memcmp(mac, vendors[i].oui, 3) == 0) {
      return vendors[i].name;
    }
  }
  return "Unknown";
}

/* ========== Distance Estimator ========== */
float estimateDistance(int rssi) {
  // Path loss model: RSSI = -10n*log10(d) + A
  // A = RSSI at 1m (calibrated at -40dBm)
  // n = path loss exponent (2.5 for indoor)
  const int A = -40;
  const float n = 2.5;

  if (rssi >= A) return 0.5; // Very close

  float ratio = (float)(A - rssi) / (10.0 * n);
  float distance = pow(10.0, ratio);

  return distance;
}

/* ========== Signal Quality Calculator ========== */
char getQualityGrade(wifi_ap_record_t* ap) {
  int score = 0;

  // RSSI scoring (0-40 points)
  if (ap->rssi >= -50) score += 40;
  else if (ap->rssi >= -60) score += 30;
  else if (ap->rssi >= -70) score += 20;
  else if (ap->rssi >= -80) score += 10;

  // Security scoring (0-30 points)
  if (ap->authmode == WIFI_AUTH_WPA3_PSK) score += 30;
  else if (ap->authmode == WIFI_AUTH_WPA2_PSK) score += 25;
  else if (ap->authmode == WIFI_AUTH_WPA_WPA2_PSK) score += 20;
  else if (ap->authmode == WIFI_AUTH_WPA_PSK) score += 10;
  else if (ap->authmode == WIFI_AUTH_WEP) score += 5;

  // Channel congestion penalty (0-30 points)
  uint8_t load = channelLoad(ap->primary);
  if (load < 20) score += 30;
  else if (load < 40) score += 20;
  else if (load < 70) score += 10;

  // Grade: A=90+, B=75+, C=60+, D=45+, F=<45
  if (score >= 90) return 'A';
  if (score >= 75) return 'B';
  if (score >= 60) return 'C';
  if (score >= 45) return 'D';
  return 'F';
}

/* ========== Channel Overlap Detection ========== */
bool hasOverlap(uint8_t ch1, uint8_t ch2) {
  // 2.4GHz channels are 5MHz apart, 20MHz wide
  // Channels overlap if within 4 channels of each other
  return abs(ch1 - ch2) <= 4 && abs(ch1 - ch2) > 0;
}

uint8_t countOverlappingAPs(uint8_t channel) {
  uint8_t count = 0;
  for (int i = 0; i < apCount; i++) {
    if (hasOverlap(channel, apList[i].primary)) {
      count++;
    }
  }
  return count;
}

/* ========== Authentication Type String ========== */
const char* authStr(wifi_auth_mode_t m) {
  switch (m) {
    case WIFI_AUTH_OPEN: return "OPEN";
    case WIFI_AUTH_WEP: return "WEP";
    case WIFI_AUTH_WPA_PSK: return "WPA";
    case WIFI_AUTH_WPA2_PSK: return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2";
    case WIFI_AUTH_WPA3_PSK: return "WPA3";
    default: return "?";
  }
}
