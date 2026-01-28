#include "device_monitor.h"
#include "wifi_scanner.h"
#include "ble_scanner.h"
#include <string.h>

/* ========== Device Monitor Implementation ========== */

void addOrUpdateWiFiDevice(const uint8_t* bssid, const char* ssid, int8_t rssi, uint8_t channel) {
  uint32_t now = millis();

  // First, try to find existing device by BSSID
  for (int i = 0; i < MAX_MONITORED_DEVICES; i++) {
    if (monitoredDevices[i].active &&
        monitoredDevices[i].type == 0 &&
        memcmp(monitoredDevices[i].bssid, bssid, 6) == 0) {
      // Update existing device
      monitoredDevices[i].rssi = rssi;
      monitoredDevices[i].channel = channel;
      monitoredDevices[i].lastSeen = now;
      monitoredDevices[i].seenCount++;
      monitoredDevices[i].isPresent = true;

      // Update name if it changed (e.g., hidden SSID now visible)
      if (ssid && strlen(ssid) > 0) {
        strncpy(monitoredDevices[i].name, ssid, 32);
        monitoredDevices[i].name[32] = '\0';
      }
      return;
    }
  }

  // Device not found, add new one
  if (monitoredDeviceCount < MAX_MONITORED_DEVICES) {
    // Find first empty slot
    for (int i = 0; i < MAX_MONITORED_DEVICES; i++) {
      if (!monitoredDevices[i].active) {
        monitoredDevices[i].type = 0;  // WiFi
        memcpy(monitoredDevices[i].bssid, bssid, 6);

        if (ssid && strlen(ssid) > 0) {
          strncpy(monitoredDevices[i].name, ssid, 32);
        } else {
          strcpy(monitoredDevices[i].name, "<hidden>");
        }
        monitoredDevices[i].name[32] = '\0';

        monitoredDevices[i].rssi = rssi;
        monitoredDevices[i].channel = channel;
        monitoredDevices[i].firstSeen = now;
        monitoredDevices[i].lastSeen = now;
        monitoredDevices[i].seenCount = 1;
        monitoredDevices[i].isPresent = true;
        monitoredDevices[i].active = true;
        monitoredDeviceCount++;
        return;
      }
    }
  }
}

void addOrUpdateBLEDevice(const char* address, const char* name, int8_t rssi) {
  uint32_t now = millis();

  // First, try to find existing device by address
  for (int i = 0; i < MAX_MONITORED_DEVICES; i++) {
    if (monitoredDevices[i].active &&
        monitoredDevices[i].type == 1 &&
        strcmp(monitoredDevices[i].bleAddr, address) == 0) {
      // Update existing device
      monitoredDevices[i].rssi = rssi;
      monitoredDevices[i].lastSeen = now;
      monitoredDevices[i].seenCount++;
      monitoredDevices[i].isPresent = true;

      // Update name if it changed
      if (name && strlen(name) > 0) {
        strncpy(monitoredDevices[i].name, name, 32);
        monitoredDevices[i].name[32] = '\0';
      }
      return;
    }
  }

  // Device not found, add new one
  if (monitoredDeviceCount < MAX_MONITORED_DEVICES) {
    // Find first empty slot
    for (int i = 0; i < MAX_MONITORED_DEVICES; i++) {
      if (!monitoredDevices[i].active) {
        monitoredDevices[i].type = 1;  // BLE
        strncpy(monitoredDevices[i].bleAddr, address, 17);
        monitoredDevices[i].bleAddr[17] = '\0';

        if (name && strlen(name) > 0) {
          strncpy(monitoredDevices[i].name, name, 32);
        } else {
          strcpy(monitoredDevices[i].name, "Unknown");
        }
        monitoredDevices[i].name[32] = '\0';

        monitoredDevices[i].rssi = rssi;
        monitoredDevices[i].channel = 0;  // N/A for BLE
        monitoredDevices[i].firstSeen = now;
        monitoredDevices[i].lastSeen = now;
        monitoredDevices[i].seenCount = 1;
        monitoredDevices[i].isPresent = true;
        monitoredDevices[i].active = true;
        monitoredDeviceCount++;
        return;
      }
    }
  }
}

void checkDeviceTimeouts() {
  uint32_t now = millis();

  for (int i = 0; i < MAX_MONITORED_DEVICES; i++) {
    if (monitoredDevices[i].active && monitoredDevices[i].isPresent) {
      // Check if device hasn't been seen recently
      if (now - monitoredDevices[i].lastSeen > DEVICE_TIMEOUT_MS) {
        monitoredDevices[i].isPresent = false;
      }
    }
  }
}

void updateDeviceMonitor() {
  // Mark all devices as not present initially (will be updated during scan)
  for (int i = 0; i < MAX_MONITORED_DEVICES; i++) {
    if (monitoredDevices[i].active) {
      monitoredDevices[i].isPresent = false;
    }
  }

  // Update from current AP scan results
  extern uint16_t apCount;
  extern wifi_ap_record_t apList[];
  for (int i = 0; i < apCount; i++) {
    addOrUpdateWiFiDevice(
      apList[i].bssid,
      (char*)apList[i].ssid,
      apList[i].rssi,
      apList[i].primary
    );
  }

  // Update from current BLE scan results
  extern uint8_t bleDeviceCount;
  extern BLEDeviceInfo bleDevices[];
  for (int i = 0; i < bleDeviceCount; i++) {
    addOrUpdateBLEDevice(
      bleDevices[i].address.c_str(),
      bleDevices[i].name.c_str(),
      bleDevices[i].rssi
    );
  }

  // Check for timeouts
  checkDeviceTimeouts();
}

void clearDeviceMonitor() {
  for (int i = 0; i < MAX_MONITORED_DEVICES; i++) {
    monitoredDevices[i].active = false;
    monitoredDevices[i].isPresent = false;
    monitoredDevices[i].seenCount = 0;
  }
  monitoredDeviceCount = 0;
  deviceCursor = 0;
  deviceScroll = 0;
}
