#ifndef BLE_SCANNER_H
#define BLE_SCANNER_H

#include "config.h"
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

/* ========== BLE Data ========== */
extern BLEDeviceInfo bleDevices[MAX_BLE_DEVICES];
extern uint8_t bleDeviceCount;
extern uint8_t bleCursor;
extern uint8_t bleScroll;
extern uint8_t bleSelectedIndex;
extern bool bleScanning;
extern bool bleInitialized;
extern uint32_t bleScanStart;
extern uint32_t lastBLEScan;
extern uint32_t lastBLESort;
extern BLEScan* pBLEScan;

/* ========== BLE Scan Callback ========== */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice);
};

/* ========== BLE Functions ========== */
void initBLE();
void startBLEScan();
void stopBLEScan();
void updateBLEScan();
void sortBLEByRSSI();
void cleanupInactiveBLE();
uint8_t getActiveBLECount();

#endif // BLE_SCANNER_H
