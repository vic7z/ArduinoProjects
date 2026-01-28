#ifndef DEVICE_MONITOR_H
#define DEVICE_MONITOR_H

#include "config.h"

/* ========== Device Monitor Functions ========== */

// Update device monitor with current scans
void updateDeviceMonitor();

// Add or update a WiFi device in the monitor
void addOrUpdateWiFiDevice(const uint8_t* bssid, const char* ssid, int8_t rssi, uint8_t channel);

// Add or update a BLE device in the monitor
void addOrUpdateBLEDevice(const char* address, const char* name, int8_t rssi);

// Mark devices as not present if they haven't been seen recently
void checkDeviceTimeouts();

// Clear all monitored devices
void clearDeviceMonitor();

#endif // DEVICE_MONITOR_H
