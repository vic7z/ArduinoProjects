#include "ble_scanner.h"
#include "wifi_scanner.h"

/* ========== BLE Data Variables ========== */
BLEDeviceInfo bleDevices[MAX_BLE_DEVICES];
uint8_t bleDeviceCount = 0;
uint8_t bleCursor = 0;
uint8_t bleScroll = 0;
uint8_t bleSelectedIndex = 0;
bool bleScanning = false;
bool bleInitialized = false;
uint32_t bleScanStart = 0;
uint32_t lastBLEScan = 0;
uint32_t lastBLESort = 0;
BLEScan* pBLEScan = nullptr;

/* ========== BLE Scan Callback ========== */
void MyAdvertisedDeviceCallbacks::onResult(BLEAdvertisedDevice advertisedDevice) {
  // Check if device already exists
  String addr = advertisedDevice.getAddress().toString().c_str();
  bool found = false;

  for (int i = 0; i < bleDeviceCount; i++) {
    if (bleDevices[i].address == addr) {
      bleDevices[i].rssi = advertisedDevice.getRSSI();
      bleDevices[i].lastSeen = millis();
      bleDevices[i].isActive = true;
      if (advertisedDevice.haveName()) {
        bleDevices[i].name = advertisedDevice.getName().c_str();
        bleDevices[i].hasName = true;
      }
      found = true;
      break;
    }
  }

  // Add new device
  if (!found && bleDeviceCount < MAX_BLE_DEVICES) {
    bleDevices[bleDeviceCount].address = addr;
    bleDevices[bleDeviceCount].rssi = advertisedDevice.getRSSI();
    bleDevices[bleDeviceCount].lastSeen = millis();
    bleDevices[bleDeviceCount].isActive = true;
    bleDevices[bleDeviceCount].advType = 0;

    if (advertisedDevice.haveName()) {
      bleDevices[bleDeviceCount].name = advertisedDevice.getName().c_str();
      bleDevices[bleDeviceCount].hasName = true;
    } else {
      bleDevices[bleDeviceCount].name = "Unknown";
      bleDevices[bleDeviceCount].hasName = false;
    }

    // Detect beacon types
    if (advertisedDevice.haveManufacturerData()) {
      String data = advertisedDevice.getManufacturerData().c_str();
      if (data.length() >= 2) {
        // iBeacon detection (Apple, 0x004C)
        if ((uint8_t)data[0] == 0x4C && (uint8_t)data[1] == 0x00) {
          bleDevices[bleDeviceCount].advType = 1;
        }
      }
    }

    bleDeviceCount++;
  }
}

/* ========== BLE Control Functions ========== */
void initBLE() {
  if (!bleInitialized) {
    Serial.println("[BLE] Initializing BLE...");
    BLEDevice::init("ESP32_Analyzer");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
    bleInitialized = true;
    Serial.println("[BLE] BLE ready");
  }
}

void startBLEScan() {
  if (bleInitialized && !bleScanning) {
    Serial.println("[BLE] Starting scan");
    stopAllWifi(); // Stop Wi-Fi before BLE
    bleScanning = true;
    bleScanStart = millis();
    lastBLEScan = 0;
  }
}

void stopBLEScan() {
  if (bleScanning && pBLEScan != nullptr) {
    Serial.println("[BLE] Stopping scan");
    pBLEScan->stop();
    pBLEScan->clearResults();
    bleScanning = false;
  }
}

void updateBLEScan() {
  if (!bleInitialized || !bleScanning || pBLEScan == nullptr) {
    return;
  }

  // Scan every 1 second to keep buttons responsive
  if (millis() - lastBLEScan > 1000) {
    pBLEScan->start(1, false); // 1 second scan, non-blocking
    lastBLEScan = millis();
  }

  // Mark old devices as inactive and clean them up
  uint32_t now = millis();
  for (int i = 0; i < bleDeviceCount; i++) {
    if (now - bleDevices[i].lastSeen > 10000) { // 10 seconds timeout
      bleDevices[i].isActive = false;
    }
  }

  // Remove inactive devices from the list
  cleanupInactiveBLE();
}

void sortBLEByRSSI() {
  for (int i = 0; i < bleDeviceCount - 1; i++) {
    for (int j = 0; j < bleDeviceCount - i - 1; j++) {
      if (bleDevices[j].rssi < bleDevices[j + 1].rssi) {
        BLEDeviceInfo tmp = bleDevices[j];
        bleDevices[j] = bleDevices[j + 1];
        bleDevices[j + 1] = tmp;
      }
    }
  }
}

void cleanupInactiveBLE() {
  // Remove inactive devices by compacting the array
  uint8_t writeIdx = 0;
  for (uint8_t readIdx = 0; readIdx < bleDeviceCount; readIdx++) {
    if (bleDevices[readIdx].isActive) {
      if (writeIdx != readIdx) {
        bleDevices[writeIdx] = bleDevices[readIdx];
      }
      writeIdx++;
    }
  }
  bleDeviceCount = writeIdx;
}

uint8_t getActiveBLECount() {
  uint8_t count = 0;
  for (int i = 0; i < bleDeviceCount; i++) {
    if (bleDevices[i].isActive) {
      count++;
    }
  }
  return count;
}
