#include "screens_handlers.h"
#include "screens_draw.h"
#include "wifi_scanner.h"
#include "ble_scanner.h"
#include "menu.h"
#include "display.h"
#include "utils.h"
#include "security.h"
#include "alerts.h"
#include "settings.h"
#include "device_monitor.h"

/* ========== External Variables ========== */
// Screen state
extern Screen currentScreen;

// Button and timing
extern uint32_t lastScan;
extern uint32_t lastSecond;
extern uint32_t lastAutoWifiScan;
extern uint32_t lastAutoBLEScan;
extern uint32_t lastEnvCheck;
extern uint32_t lastBaselineUpdate;
extern uint32_t lastBLESort;

// AP Scanner state
extern uint8_t apCursor, apScroll, apSelectedIndex;
extern uint8_t apCompareA, apCompareB;
extern bool apSortedOnce;

// BLE Scanner state
extern uint8_t bleCursor, bleScroll, bleSelectedIndex;

// Monitor state
extern uint8_t currentChannel, selectedChannel, analyzerChannel;
extern bool frozen;
extern volatile uint32_t pktTotal, pktBeacon, pktData, pktDeauth;
extern uint32_t lastPkt;
extern float smoothPps;
extern uint32_t pps, peak, peakPPS;
extern uint32_t history[HISTORY_SIZE];
extern uint8_t histIdx;
extern float avgRssi;
extern volatile int32_t rssiAccum;
extern volatile uint32_t rssiCount;
extern uint32_t analyzerLastHop;
extern uint32_t chPackets[MAX_CHANNEL + 1];
extern uint32_t chBeacons[MAX_CHANNEL + 1];
extern uint32_t chDeauth[MAX_CHANNEL + 1];

// Hidden SSID state
extern uint8_t hiddenCursor, hiddenScroll;

// Auto watch state
extern uint32_t autoTotalAPs, autoTotalBLE;
extern uint8_t autoModeView;

// Walk test state
extern bool walkTestActive;
extern char walkTargetSSID[33];
extern uint8_t walkTargetBSSID[6];
extern String walkTargetBLEAddr;
extern int8_t walkRSSIHistory[WALK_HISTORY_SIZE];
extern uint8_t walkHistoryIndex;
extern int8_t walkMinRSSI, walkMaxRSSI;
extern int32_t walkRSSISum;
extern uint16_t walkSampleCount;
extern uint8_t walkTestView;

// Insights state
extern uint8_t whySlowView;
extern RSSIHistory rssiHistory[MAX_TRACKED_APS];
extern uint32_t lastRSSISample;

// Security state
extern uint32_t deauthPerSecond, totalDeauthDetected;
extern bool attackActive, prevAttackActive;
extern uint8_t prevRogueCount;
extern uint8_t deauthChannel;
extern uint8_t alertLevel;

// Event log state
extern uint8_t eventCursor, eventScroll;

// Display settings state
extern uint8_t displaySettingCursor;

/* ========== Menu Handlers ========== */

void handleMainMenu(ButtonEvent ev) {
  if (ev == BTN_SHORT) {
    mainMenuIndex = (mainMenuIndex + 1) % MAIN_MENU_SIZE;
    drawMenu();
  }

  if (ev == BTN_LONG) {
    switch (mainMenuIndex) {
      case 0: // Auto Watch
        currentScreen = SCREEN_AUTO_WATCH;
        autoModeView = 0;
        lastAutoWifiScan = 0;
        lastAutoBLEScan = 0;
        autoTotalAPs = 0;
        autoTotalBLE = 0;
        currentChannel = 1;
        resetLiveStats();
        enterSnifferMode(currentChannel);
        startBLEScan();
        drawAutoWatch();
        break;
      case 1: // RF Health
        currentScreen = SCREEN_RF_HEALTH;
        stopAllWifi();
        break;
      case 2: // Live Monitor
        currentScreen = SCREEN_MONITOR;
        frozen = false;
        currentChannel = 1;
        resetLiveStats();
        enterSnifferMode(currentChannel);
        break;
      case 3: // Channel Analyzer
        currentScreen = SCREEN_ANALYZER;
        selectedChannel = 1;
        resetAnalyzer();
        enterSnifferMode(1);
        break;
      case 4: // Device Monitor
        currentScreen = SCREEN_DEVICE_MONITOR;
        deviceCursor = 0;
        deviceScroll = 0;
        lastScan = 0;  // Force immediate scan
        startBLEScan();
        break;
      case 5: // AP Scanner (direct to AP List)
        currentScreen = SCREEN_AP_LIST;
        apCursor = apScroll = 0;
        apSortedOnce = false;
        enterScanMode();
        startApScan();
        lastScan = millis();
        break;
      case 6: // BLE Monitor (direct to BLE Scan)
        currentScreen = SCREEN_BLE_SCAN;
        bleCursor = bleScroll = 0;
        stopAllWifi();
        startBLEScan();
        lastScan = millis();
        break;
      case 7: // Security
        currentScreen = SCREEN_SECURITY_MENU;
        securityMenuIndex = 0;
        stopAllWifi();
        drawSecurityMenu();
        break;
      case 8: // Insights
        currentScreen = SCREEN_INSIGHTS_MENU;
        insightsMenuIndex = 0;
        stopAllWifi();
        drawInsightsMenu();
        break;
      case 9: // History
        currentScreen = SCREEN_HISTORY_MENU;
        historyMenuIndex = 0;
        stopAllWifi();
        drawHistoryMenu();
        break;
      case 10: // System
        currentScreen = SCREEN_SYSTEM_MENU;
        systemMenuIndex = 0;
        stopAllWifi();
        drawSystemMenu();
        break;
    }
  }
}

void handleSecurityMenu(ButtonEvent ev) {
  if (ev == BTN_SHORT) {
    securityMenuIndex = (securityMenuIndex + 1) % SECURITY_MENU_SIZE;
    drawSecurityMenu();
  }
  if (ev == BTN_LONG) {
    switch (securityMenuIndex) {
      case 0: // Deauth Watch
        currentScreen = SCREEN_DEAUTH_WATCH;
        currentChannel = 1;
        enterSnifferMode(currentChannel);
        break;
      case 1: // Rogue AP Watch
        currentScreen = SCREEN_ROGUE_AP_WATCH;
        enterScanMode();
        startApScan();
        lastScan = millis();
        break;
      case 2: // BLE Tracker Watch
        currentScreen = SCREEN_BLE_TRACKER_WATCH;
        break;
      case 3: // Alert Settings
        currentScreen = SCREEN_ALERT_SETTINGS;
        break;
    }
  }
  if (ev == BTN_BACK) {
    currentScreen = SCREEN_MENU;
    drawMenu();
  }
}

void handleInsightsMenu(ButtonEvent ev) {
  if (ev == BTN_SHORT) {
    insightsMenuIndex = (insightsMenuIndex + 1) % INSIGHTS_MENU_SIZE;
    drawInsightsMenu();
  }
  if (ev == BTN_LONG) {
    switch (insightsMenuIndex) {
      case 0: // Why Is It Slow?
        currentScreen = SCREEN_WHY_IS_IT_SLOW;
        break;
      case 1: // Channel Recommendation
        currentScreen = SCREEN_CHANNEL_RECOMMENDATION;
        lastScan = 0;  // Force immediate scan
        break;
      case 2: // Environment Change
        currentScreen = SCREEN_ENVIRONMENT_CHANGE;
        lastEnvCheck = 0;  // Force immediate scan
        break;
      case 3: // Quick Snapshot
        currentScreen = SCREEN_QUICK_SNAPSHOT;
        lastScan = 0;  // Force immediate scan
        break;
      case 4: // Channel Scorecard
        currentScreen = SCREEN_CHANNEL_SCORECARD;
        lastScan = 0;  // Force immediate scan
        break;
    }
  }
  if (ev == BTN_BACK) {
    currentScreen = SCREEN_MENU;
    drawMenu();
  }
}

void handleHistoryMenu(ButtonEvent ev) {
  if (ev == BTN_SHORT) {
    historyMenuIndex = (historyMenuIndex + 1) % HISTORY_MENU_SIZE;
    drawHistoryMenu();
  }
  if (ev == BTN_LONG) {
    switch (historyMenuIndex) {
      case 0: // Event Log
        currentScreen = SCREEN_EVENT_LOG;
        break;
      case 1: // Baseline Compare
        currentScreen = SCREEN_BASELINE_COMPARE;
        lastBaselineUpdate = 0;  // Force immediate scan
        break;
      case 2: // Export Data
        currentScreen = SCREEN_EXPORT;
        lastScan = 0;  // Force immediate scan for fresh data
        break;
    }
  }
  if (ev == BTN_BACK) {
    currentScreen = SCREEN_MENU;
    drawMenu();
  }
}

void handleSystemMenu(ButtonEvent ev) {
  if (ev == BTN_SHORT) {
    systemMenuIndex = (systemMenuIndex + 1) % SYSTEM_MENU_SIZE;
    drawSystemMenu();
  }
  if (ev == BTN_LONG) {
    switch (systemMenuIndex) {
      case 0: // Battery & Power
        currentScreen = SCREEN_BATTERY_POWER;
        break;
      case 1: // Display
        currentScreen = SCREEN_DISPLAY_SETTINGS;
        break;
      case 2: // Radio Control
        currentScreen = SCREEN_RADIO_CONTROL;
        break;
      case 3: // Power Mode
        currentScreen = SCREEN_POWER_MODE;
        break;
      case 4: // About
        currentScreen = SCREEN_ABOUT;
        break;
    }
  }
  if (ev == BTN_BACK) {
    currentScreen = SCREEN_MENU;
    drawMenu();
  }
}

/* ========== Main Screen Handlers ========== */

void handleAutoWatch(ButtonEvent ev) {
  // Handle buttons first for better responsiveness
  if (ev == BTN_SHORT || ev == BTN_LONG) {
    autoModeView = (autoModeView + 1) % 4;  // 4 views: Summary, Top APs, Top BLE, Channel APs
    Serial.printf("[AUTO] View: %d\n", autoModeView);
    drawAutoWatch();
    return;
  }

  if (ev == BTN_BACK) {
    currentScreen = SCREEN_MENU;
    stopAllWifi();
    stopBLEScan();
    drawMenu();
    return;
  }

  // Update BLE scan
  updateBLEScan();
  cleanupInactiveBLE();  // Ensure cleanup happens
  if (bleDeviceCount > 0) {
    sortBLEByRSSI();
  }
  autoTotalBLE = bleDeviceCount;  // After cleanup, bleDeviceCount is accurate

  // Periodic AP scan (every 5 seconds)
  if (millis() - lastAutoWifiScan > 5000) {
    enterScanMode();
    startApScan();
    delay(1500); // Wait for scan to complete (WiFi scans take ~1-2 seconds)
    fetchApResults(true);
    autoTotalAPs = apCount;
    lastAutoWifiScan = millis();
    // Return to sniffer mode and ensure BLE scanning continues
    enterSnifferMode(currentChannel);
    startBLEScan();
  }

  // Cycle channel for sniffing
  if (millis() - lastSecond > 1000) {
    currentChannel = (currentChannel % 11) + 1;
    enterSnifferMode(currentChannel);
    startBLEScan(); // Restart BLE scan after channel switch
    lastSecond = millis();
  }

  drawAutoWatch();
}

void handleRFHealth(ButtonEvent ev) {
  // Scan periodically for fresh data
  if (millis() - lastScan > 3000) {
    enterScanMode();
    startApScan();
    delay(1500);  // Wait for scan to complete (WiFi scans take ~1-2 seconds)
    fetchApResults(false);
    updateBLEScan(); // Update BLE devices
    lastScan = millis();
  }

  if (ev == BTN_BACK) {
    currentScreen = SCREEN_MENU;
    stopAllWifi();
    stopBLEScan();
    drawMenu();
    return;
  }

  drawRFHealth();
}

void handleMonitor(ButtonEvent ev) {
  if (ev == BTN_SHORT && !frozen) {
    currentChannel = currentChannel % MAX_CHANNEL + 1;
    resetLiveStats();
    enterSnifferMode(currentChannel);
  }

  if (ev == BTN_LONG) {
    frozen = !frozen;
  }

  if (ev == BTN_BACK) {
    currentScreen = SCREEN_MENU;
    stopAllWifi();
    drawMenu();
    return;
  }

  if (!frozen && millis() - lastSecond >= 1000) {
    uint32_t d = pktTotal - lastPkt;
    lastPkt = pktTotal;
    smoothPps = 0.7 * smoothPps + 0.3 * d;
    pps = (uint32_t)smoothPps;
    peak = max(peak, pps);
    peakPPS = max(peakPPS, pps);
    history[histIdx] = pps;
    histIdx = (histIdx + 1) % HISTORY_SIZE;

    if (rssiCount) {
      float r = (float)rssiAccum / rssiCount;
      avgRssi = 0.8 * avgRssi + 0.2 * r;
      rssiAccum = rssiCount = 0;
    }
    lastSecond = millis();
  }

  drawMonitor();
}

void handleAnalyzer(ButtonEvent ev) {
  uint32_t hopDelay = 30;
  if (settings.scanSpeed == 0) hopDelay = 15;      // Fast
  else if (settings.scanSpeed == 2) hopDelay = 50;  // Slow

  if (millis() - analyzerLastHop > hopDelay) {
    chPackets[analyzerChannel] += pktTotal;
    chBeacons[analyzerChannel] += pktBeacon;
    chDeauth[analyzerChannel] += pktDeauth;

    pktTotal = pktBeacon = pktDeauth = 0;
    analyzerChannel = analyzerChannel % MAX_CHANNEL + 1;
    enterSnifferMode(analyzerChannel);
    analyzerLastHop = millis();
  }

  if (ev == BTN_SHORT) {
    selectedChannel = selectedChannel % MAX_CHANNEL + 1;
  }

  if (ev == BTN_LONG) {
    currentChannel = selectedChannel;
    currentScreen = SCREEN_MONITOR;
    frozen = false;
    resetLiveStats();
    enterSnifferMode(currentChannel);
  }

  if (ev == BTN_BACK) {
    currentScreen = SCREEN_MENU;
    stopAllWifi();
    drawMenu();
    return;
  }

  drawAnalyzer();
}

void handleDeviceMonitor(ButtonEvent ev) {
  // Handle buttons FIRST for better responsiveness
  if (ev == BTN_SHORT && monitoredDeviceCount > 0) {
    // Count active devices
    uint8_t activeCount = 0;
    for (int i = 0; i < MAX_MONITORED_DEVICES; i++) {
      if (monitoredDevices[i].active) activeCount++;
    }

    if (deviceScroll + deviceCursor + 1 < activeCount) {
      if (deviceCursor < 2) deviceCursor++;  // Show 3 devices at a time
      else deviceScroll++;
    } else {
      deviceCursor = 0;
      deviceScroll = 0;
    }
  }

  if (ev == BTN_LONG && monitoredDeviceCount > 0) {
    // Find the actual device index (skipping inactive slots)
    uint8_t activeIndex = 0;
    for (int i = 0; i < MAX_MONITORED_DEVICES; i++) {
      if (monitoredDevices[i].active) {
        if (activeIndex == deviceScroll + deviceCursor) {
          deviceSelectedIndex = i;
          currentScreen = SCREEN_DEVICE_DETAIL;
          drawDeviceDetail();
          return;
        }
        activeIndex++;
      }
    }
  }

  if (ev == BTN_BACK) {
    currentScreen = SCREEN_MENU;
    stopAllWifi();
    stopBLEScan();
    drawMenu();
    return;
  }

  // Scan periodically for device updates (2 seconds for faster updates)
  if (millis() - lastScan > 2000) {
    enterScanMode();
    startApScan();
    delay(1500); // Wait for scan to complete
    fetchApResults(false);
    updateBLEScan();
    updateDeviceMonitor();
    lastScan = millis();
  }

  drawDeviceMonitor();
}

void handleDeviceDetail(ButtonEvent ev) {
  drawDeviceDetail();
  if (ev == BTN_BACK) {
    currentScreen = SCREEN_DEVICE_MONITOR;
    drawDeviceMonitor();
  }
}

/* ========== AP Scanner Handlers ========== */

void handleApList(ButtonEvent ev) {
  // Handle buttons FIRST for better responsiveness
  if (ev == BTN_SHORT && apCount > 0) {
    if (apScroll + apCursor + 1 < apCount) {
      if (apCursor < AP_VISIBLE - 1) apCursor++;
      else apScroll++;
    } else {
      apCursor = 0;
      apScroll = 0;
    }
  }

  if (ev == BTN_LONG && apCount > 0) {
    apSelectedIndex = apScroll + apCursor;
    currentScreen = SCREEN_AP_DETAIL;
  }

  if (ev == BTN_BACK) {
    currentScreen = SCREEN_MENU;
    stopAllWifi();
    drawMenu();
    return;
  }

  // Scan operations after button handling
  if (millis() - lastScan > 2000) {
    fetchApResults(false);
    startApScan();
    lastScan = millis();
  }

  drawApList();
}

void handleApDetail(ButtonEvent ev) {
  if (ev == BTN_SHORT) {
    currentScreen = SCREEN_AP_LIST;
  }
  if (ev == BTN_LONG) {
    // Start Walk Test for this AP - save BSSID and SSID
    if (apSelectedIndex < apCount) {
      memcpy(walkTargetBSSID, apList[apSelectedIndex].bssid, 6);
      strncpy(walkTargetSSID, (char*)apList[apSelectedIndex].ssid, 32);
      walkTargetSSID[32] = '\0';

      walkHistoryIndex = 0;
      walkMinRSSI = 0;
      walkMaxRSSI = -100;
      walkRSSISum = 0;
      walkSampleCount = 0;
      walkTestActive = true;
      memset(walkRSSIHistory, 0, sizeof(walkRSSIHistory));
      currentScreen = SCREEN_AP_WALK_TEST;
      lastScan = 0; // Force immediate scan
    }
  }
  if (ev == BTN_BACK) {
    currentScreen = SCREEN_AP_LIST;
    drawApList();
  }
  drawApDetail();
}

void handleAPWalkTest(ButtonEvent ev) {
  // Toggle view on short press
  if (ev == BTN_SHORT) {
    walkTestView = (walkTestView + 1) % 2;
    delay(50); // Small delay to debounce
    drawAPWalkTest();
    return;
  }

  // Scan for target AP frequently (every 1.5 seconds for updates)
  if (millis() - lastScan > 1500) {
    enterScanMode();
    startApScan();
    delay(1500); // Wait for scan to complete
    fetchApResults(false);

    // Find target AP by BSSID and update walk test stats
    if (apCount > 0) {
      for (int i = 0; i < apCount; i++) {
        if (memcmp(apList[i].bssid, walkTargetBSSID, 6) == 0) {
          int8_t rssi = apList[i].rssi;

          // Add to history
          walkRSSIHistory[walkHistoryIndex] = rssi;
          walkHistoryIndex = (walkHistoryIndex + 1) % WALK_HISTORY_SIZE;

          // Update min/max
          if (walkSampleCount == 0 || rssi < walkMinRSSI) walkMinRSSI = rssi;
          if (walkSampleCount == 0 || rssi > walkMaxRSSI) walkMaxRSSI = rssi;

          // Update average
          walkRSSISum += rssi;
          walkSampleCount++;
          break;
        }
      }
    }

    lastScan = millis();
  }

  if (ev == BTN_BACK) {
    currentScreen = SCREEN_AP_DETAIL;
    stopAllWifi();
    walkTestActive = false;
    walkTestView = 0; // Reset view
    drawApDetail();
    return;
  }

  drawAPWalkTest();
}

void handleCompare(ButtonEvent ev) {
  if (ev == BTN_SHORT) {
    apCompareB = (apCompareB + 1) % apCount;
  }
  if (ev == BTN_LONG) {
    apCompareA = (apCompareA + 1) % apCount;
    if (apCompareA == apCompareB) apCompareB = (apCompareB + 1) % apCount;
  }
  if (ev == BTN_BACK) {
    currentScreen = SCREEN_MENU;
    drawMenu();
  }
  drawCompare();
}

void handleHiddenSSID(ButtonEvent ev) {
  static uint32_t lastHop = 0;
  if (millis() - lastHop > 200) {
    currentChannel = (currentChannel % MAX_CHANNEL) + 1;
    enterSnifferMode(currentChannel);
    lastHop = millis();
  }

  if (ev == BTN_SHORT && hiddenCount > 0) {
    if (hiddenScroll + hiddenCursor + 1 < hiddenCount) {
      if (hiddenCursor < AP_VISIBLE - 1) hiddenCursor++;
      else hiddenScroll++;
    } else {
      hiddenCursor = 0;
      hiddenScroll = 0;
    }
  }

  if (ev == BTN_BACK) {
    currentScreen = SCREEN_MENU;
    stopAllWifi();
    drawMenu();
  }

  drawHiddenSSID();
}

/* ========== BLE Scanner Handlers ========== */

void handleBLEScan(ButtonEvent ev) {
  // Handle buttons FIRST for better responsiveness
  if (ev == BTN_SHORT && bleDeviceCount > 0) {
    if (bleScroll + bleCursor + 1 < bleDeviceCount) {
      if (bleCursor < BLE_VISIBLE - 1) bleCursor++;
      else bleScroll++;
    } else {
      bleCursor = 0;
      bleScroll = 0;
    }
  }

  if (ev == BTN_LONG && bleDeviceCount > 0) {
    bleSelectedIndex = bleScroll + bleCursor;
    currentScreen = SCREEN_BLE_DETAIL;
    stopBLEScan();
    drawBLEDetail();
    return;
  }

  if (ev == BTN_BACK) {
    currentScreen = SCREEN_MENU;
    stopBLEScan();
    drawMenu();
    return;
  }

  // Update BLE scan after button handling (non-blocking)
  updateBLEScan();

  // Sort devices periodically (every 2 seconds) to prevent list shuffling during scrolling
  if (bleDeviceCount > 1 && millis() - lastBLESort > 2000) {
    sortBLEByRSSI();
    lastBLESort = millis();

    // Validate cursor/scroll positions after sorting
    if (bleScroll + bleCursor >= bleDeviceCount) {
      if (bleDeviceCount > BLE_VISIBLE) {
        bleScroll = bleDeviceCount - BLE_VISIBLE;
        bleCursor = BLE_VISIBLE - 1;
      } else {
        bleScroll = 0;
        bleCursor = bleDeviceCount > 0 ? bleDeviceCount - 1 : 0;
      }
    }
  }

  // Validate bounds
  if (bleScroll + bleCursor >= bleDeviceCount && bleDeviceCount > 0) {
    bleCursor = 0;
    bleScroll = 0;
  }

  // Redraw screen
  drawBLEScan();

  yield(); // Feed watchdog
}

void handleBLEDetail(ButtonEvent ev) {
  if (ev == BTN_SHORT) {
    currentScreen = SCREEN_BLE_SCAN;
    startBLEScan();
  }

  if (ev == BTN_LONG) {
    // Start Walk Test for this BLE device - save address
    if (bleSelectedIndex < bleDeviceCount) {
      walkTargetBLEAddr = bleDevices[bleSelectedIndex].address;

      walkHistoryIndex = 0;
      walkMinRSSI = 0;
      walkMaxRSSI = -100;
      walkRSSISum = 0;
      walkSampleCount = 0;
      walkTestActive = true;
      memset(walkRSSIHistory, 0, sizeof(walkRSSIHistory));
      currentScreen = SCREEN_BLE_WALK_TEST;
      startBLEScan(); // Ensure BLE scanning is active
      lastScan = 0; // Force immediate update
    }
  }

  if (ev == BTN_BACK) {
    currentScreen = SCREEN_BLE_SCAN;
    startBLEScan();
  }

  drawBLEDetail();
}

void handleBLEWalkTest(ButtonEvent ev) {
  // Toggle view on short press
  if (ev == BTN_SHORT) {
    walkTestView = (walkTestView + 1) % 2;
    delay(50); // Small delay to debounce
    drawBLEWalkTest();
    return;
  }

  // Update BLE scan frequently (every 500ms for fast updates)
  updateBLEScan();

  if (millis() - lastScan > 500) {
    // Find target BLE device by address and update walk test stats
    if (bleDeviceCount > 0 && walkTargetBLEAddr.length() > 0) {
      for (int i = 0; i < bleDeviceCount; i++) {
        if (bleDevices[i].address == walkTargetBLEAddr) {
          int8_t rssi = bleDevices[i].rssi;

          // Add to history
          walkRSSIHistory[walkHistoryIndex] = rssi;
          walkHistoryIndex = (walkHistoryIndex + 1) % WALK_HISTORY_SIZE;

          // Update min/max
          if (walkSampleCount == 0 || rssi < walkMinRSSI) walkMinRSSI = rssi;
          if (walkSampleCount == 0 || rssi > walkMaxRSSI) walkMaxRSSI = rssi;

          // Update average
          walkRSSISum += rssi;
          walkSampleCount++;
          break;
        }
      }
    }

    lastScan = millis();
  }

  if (ev == BTN_BACK) {
    currentScreen = SCREEN_BLE_DETAIL;
    stopBLEScan();
    walkTestView = 0; // Reset view
    drawBLEDetail();
    return;
  }

  drawBLEWalkTest();
}

/* ========== Security Handlers ========== */

void handleDeauthWatch(ButtonEvent ev) {
  // Channel hopping - scan all channels for deauth packets
  static uint32_t lastChannelHop = 0;
  if (millis() - lastChannelHop > 500) {  // Hop every 500ms
    currentChannel = (currentChannel % MAX_CHANNEL) + 1;
    enterSnifferMode(currentChannel);
    lastChannelHop = millis();
  }

  // Update total deauth counter
  totalDeauthDetected = pktDeauth;

  // Update alert level based on attack status
  if (attackActive) {
    alertLevel = 2;  // Critical - red blink
  } else if (deauthPerSecond > 0) {
    alertLevel = 1;  // Warning - orange blink
  } else {
    alertLevel = 0;  // Normal - green
  }
  updateAlertLED();

  // Log attack events
  if (attackActive && !prevAttackActive) {
    char msg[40];
    snprintf(msg, 40, "Deauth attack! Ch%d %lu/sec", currentChannel, deauthPerSecond);
    logEvent(0, msg);
  }
  prevAttackActive = attackActive;

  drawDeauthWatch();
  if (ev == BTN_BACK) {
    currentScreen = SCREEN_SECURITY_MENU;
    stopAllWifi();
    drawSecurityMenu();
    alertLevel = 0;  // Reset to normal
    setRGB(RGB_GREEN);
  }
}

void handleRogueAPWatch(ButtonEvent ev) {
  if (millis() - lastScan > 3000) {
    fetchApResults(false);
    uint8_t oldRogueCount = rogueCount;
    detectRogueAPs();  // Check for APs with same SSID but different BSSID

    // Log new rogue detections
    if (rogueCount > prevRogueCount) {
      for (int i = prevRogueCount; i < rogueCount; i++) {
        char msg[40];
        snprintf(msg, 40, "Rogue AP: %.15s", rogueList[i].ssid);
        logEvent(1, msg);
      }
    }
    prevRogueCount = rogueCount;

    startApScan();
    lastScan = millis();
  }

  // Update alert level based on rogue AP detection
  if (rogueCount > 0) {
    alertLevel = 2;  // Critical - red blink
  } else {
    alertLevel = 0;  // Normal - green
  }
  updateAlertLED();

  drawRogueAPWatch();
  if (ev == BTN_BACK) {
    currentScreen = SCREEN_SECURITY_MENU;
    stopAllWifi();
    drawSecurityMenu();
    alertLevel = 0;  // Reset to normal
    setRGB(RGB_GREEN);
  }
}

void handleBLETrackerWatch(ButtonEvent ev) {
  // Update BLE scan
  updateBLEScan();

  // Count potential trackers
  int trackerCount = 0;
  for (int i = 0; i < bleDeviceCount; i++) {
    if (bleDevices[i].advType > 0 || !bleDevices[i].hasName) {
      trackerCount++;
    }
  }

  // Update alert level based on tracker detection
  if (trackerCount > 0) {
    alertLevel = 1;  // Warning - orange blink
  } else {
    alertLevel = 0;  // Normal - green
  }
  updateAlertLED();

  drawBLETrackerWatch();
  if (ev == BTN_BACK) {
    currentScreen = SCREEN_SECURITY_MENU;
    stopBLEScan();
    drawSecurityMenu();
    alertLevel = 0;  // Reset to normal
    setRGB(RGB_GREEN);
  }
}

void handleAlertSettings(ButtonEvent ev) {
  if (ev == BTN_SHORT) {
    // Navigate between settings
    alertSettingIndex = (alertSettingIndex + 1) % 2;
  } else if (ev == BTN_LONG) {
    // Adjust selected setting
    switch (alertSettingIndex) {
      case 0:  // Deauth threshold
        settings.deauthThreshold = (settings.deauthThreshold + 5) % 55;  // 0, 5, 10, ... 50
        if (settings.deauthThreshold == 0) settings.deauthThreshold = 5;
        break;
      case 1:  // Screen timeout
        if (settings.screenTimeout == 0) settings.screenTimeout = 30;
        else if (settings.screenTimeout == 30) settings.screenTimeout = 60;
        else if (settings.screenTimeout == 60) settings.screenTimeout = 120;
        else if (settings.screenTimeout == 120) settings.screenTimeout = 300;
        else settings.screenTimeout = 0;  // Never
        break;
    }
  }

  drawAlertSettings();

  if (ev == BTN_BACK) {
    saveSettings();  // Save settings on exit
    currentScreen = SCREEN_SECURITY_MENU;
    drawSecurityMenu();
  }
}

/* ========== Insights Handlers ========== */

void updateRSSIHistory() {
  // Track top 3 APs by RSSI
  if (apCount == 0) return;

  // Get top 3 APs by RSSI
  uint8_t topIndices[MAX_TRACKED_APS] = {0, 0, 0};
  int8_t topRSSI[MAX_TRACKED_APS] = {-128, -128, -128};

  for (int i = 0; i < apCount && i < MAX_APS; i++) {
    for (int j = 0; j < MAX_TRACKED_APS; j++) {
      if (apList[i].rssi > topRSSI[j]) {
        // Shift down lower entries
        for (int k = MAX_TRACKED_APS - 1; k > j; k--) {
          topRSSI[k] = topRSSI[k - 1];
          topIndices[k] = topIndices[k - 1];
        }
        topRSSI[j] = apList[i].rssi;
        topIndices[j] = i;
        break;
      }
    }
  }

  // Update history for each tracked AP
  for (int i = 0; i < MAX_TRACKED_APS; i++) {
    if (topRSSI[i] > -128) {
      uint8_t apIdx = topIndices[i];

      // Check if this AP is already being tracked
      bool found = false;
      for (int j = 0; j < MAX_TRACKED_APS; j++) {
        if (rssiHistory[j].active &&
            memcmp(rssiHistory[j].bssid, apList[apIdx].bssid, 6) == 0) {
          // Update existing entry
          rssiHistory[j].rssiSamples[rssiHistory[j].sampleIndex] = apList[apIdx].rssi;
          rssiHistory[j].sampleIndex = (rssiHistory[j].sampleIndex + 1) % RSSI_HISTORY_SIZE;
          found = true;
          break;
        }
      }

      // If not found, add to first available slot
      if (!found) {
        for (int j = 0; j < MAX_TRACKED_APS; j++) {
          if (!rssiHistory[j].active) {
            memcpy(rssiHistory[j].bssid, apList[apIdx].bssid, 6);
            memset(rssiHistory[j].rssiSamples, -100, RSSI_HISTORY_SIZE);
            rssiHistory[j].rssiSamples[0] = apList[apIdx].rssi;
            rssiHistory[j].sampleIndex = 1;
            rssiHistory[j].active = true;
            break;
          }
        }
      }
    }
  }
}

void handleWhyIsItSlow(ButtonEvent ev) {
  // Toggle view on long press
  if (ev == BTN_LONG) {
    whySlowView = (whySlowView + 1) % 2;
  }

  if (ev == BTN_BACK) {
    currentScreen = SCREEN_INSIGHTS_MENU;
    stopAllWifi();
    whySlowView = 0; // Reset to analysis view
    drawInsightsMenu();
    return;
  }

  // Periodically scan and collect RSSI samples
  if (millis() - lastScan > 2000) {
    enterScanMode();
    startApScan();
    delay(1500);
    fetchApResults(false);
    updateRSSIHistory();
    lastScan = millis();
  }

  drawWhyIsItSlow();
}

void handleChannelRecommendation(ButtonEvent ev) {
  // Channel Recommendation needs fresh scan data
  if (millis() - lastScan > 3000) {
    enterScanMode();
    startApScan();
    delay(1500); // Wait for scan to complete
    fetchApResults(false);
    lastScan = millis();
  }
  drawChannelRecommendation();

  if (ev == BTN_BACK) {
    currentScreen = SCREEN_INSIGHTS_MENU;
    stopAllWifi();
    drawInsightsMenu();
  }
}

void handleEnvironmentChange(ButtonEvent ev) {
  // Update snapshot periodically
  if (millis() - lastEnvCheck > 2000) {
    enterScanMode();
    startApScan();
    delay(1500); // Wait for scan to complete
    fetchApResults(false);
    takeSnapshot(&currentSnapshot);
    lastEnvCheck = millis();
  }
  drawEnvironmentChange();

  // Long press to save baseline
  if (ev == BTN_LONG) {
    baseline = currentSnapshot;
    logEvent(1, "Baseline saved");
  }

  if (ev == BTN_BACK) {
    currentScreen = SCREEN_INSIGHTS_MENU;
    stopAllWifi();
    drawInsightsMenu();
  }
}

void handleQuickSnapshot(ButtonEvent ev) {
  // Scan for fresh data
  if (millis() - lastScan > 2000) {
    enterScanMode();
    startApScan();
    delay(1500);
    fetchApResults(false);
    updateBLEScan();
    lastScan = millis();
  }

  drawQuickSnapshot();

  if (ev == BTN_BACK) {
    currentScreen = SCREEN_INSIGHTS_MENU;
    stopAllWifi();
    drawInsightsMenu();
    return;
  }
}

void handleChannelScorecard(ButtonEvent ev) {
  // Scan for fresh data
  if (millis() - lastScan > 2000) {
    enterScanMode();
    startApScan();
    delay(1500);
    fetchApResults(false);
    lastScan = millis();
  }

  drawChannelScorecard();

  if (ev == BTN_BACK) {
    currentScreen = SCREEN_INSIGHTS_MENU;
    stopAllWifi();
    drawInsightsMenu();
    return;
  }
}

/* ========== History Handlers ========== */

void handleEventLog(ButtonEvent ev) {
  drawEventLog();

  if (ev == BTN_BACK) {
    currentScreen = SCREEN_HISTORY_MENU;
    drawHistoryMenu();
  }
}

void handleBaselineCompare(ButtonEvent ev) {
  // Update current snapshot periodically
  if (millis() - lastBaselineUpdate > 2000) {
    enterScanMode();
    startApScan();
    delay(1500); // Wait for scan to complete
    fetchApResults(false);
    takeSnapshot(&currentSnapshot);
    lastBaselineUpdate = millis();
  }

  drawBaselineCompare();

  if (ev == BTN_BACK) {
    currentScreen = SCREEN_HISTORY_MENU;
    stopAllWifi();
    drawHistoryMenu();
  }
}

/* ========== System Handlers ========== */

void handleBatteryPower(ButtonEvent ev) {
  drawBatteryPower();

  if (ev == BTN_BACK) {
    currentScreen = SCREEN_SYSTEM_MENU;
    drawSystemMenu();
  }
}

void handleDisplaySettings(ButtonEvent ev) {
  if (ev == BTN_SHORT) {
    // Cycle through settings
    displaySettingCursor = (displaySettingCursor + 1) % 2;
  } else if (ev == BTN_LONG) {
    // Adjust current setting
    if (displaySettingCursor == 0) {
      // RGB Brightness: cycle through values
      settings.rgbBrightness = (settings.rgbBrightness + 10) % 110;
      if (settings.rgbBrightness > 100) settings.rgbBrightness = 0;
      rgb.setBrightness((settings.rgbBrightness * 255) / 100);
      rgb.show();
    } else if (displaySettingCursor == 1) {
      // Sleep Timeout: 0(Never), 30, 60, 120, 300
      if (settings.screenTimeout == 0) settings.screenTimeout = 30;
      else if (settings.screenTimeout == 30) settings.screenTimeout = 60;
      else if (settings.screenTimeout == 60) settings.screenTimeout = 120;
      else if (settings.screenTimeout == 120) settings.screenTimeout = 300;
      else settings.screenTimeout = 0;
    }
  }

  drawDisplaySettings();

  if (ev == BTN_BACK) {
    saveSettings();  // Save on exit
    displaySettingCursor = 0; // Reset cursor
    currentScreen = SCREEN_SYSTEM_MENU;
    drawSystemMenu();
  }
}

void handleRadioControl(ButtonEvent ev) {
  // Change channel
  if (ev == BTN_SHORT) {
    currentChannel = (currentChannel % MAX_CHANNEL) + 1;
  } else if (ev == BTN_LONG) {
    currentChannel = (currentChannel - 2 + MAX_CHANNEL) % MAX_CHANNEL + 1;
  }

  drawRadioControl();

  if (ev == BTN_BACK) {
    currentScreen = SCREEN_SYSTEM_MENU;
    drawSystemMenu();
  }
}

void handleAbout(ButtonEvent ev) {
  drawAbout();

  if (ev == BTN_BACK) {
    currentScreen = SCREEN_SYSTEM_MENU;
    drawSystemMenu();
  }
}

void handlePowerMode(ButtonEvent ev) {
  // Cycle through power modes
  if (ev == BTN_SHORT) {
    settings.powerMode = (settings.powerMode + 1) % 3;
  }

  drawPowerMode();

  if (ev == BTN_BACK) {
    saveSettings();  // Save on exit
    currentScreen = SCREEN_SYSTEM_MENU;
    drawSystemMenu();
    return;
  }
}

void handleExport(ButtonEvent ev) {
  // Export to serial on short press
  if (ev == BTN_SHORT) {
    Serial.println("\n========== DATA EXPORT ==========");
    Serial.printf("Timestamp: %lu ms\n\n", millis());

    // Export WiFi APs
    Serial.printf("WiFi Networks: %d\n", apCount);
    for (int i = 0; i < apCount; i++) {
      Serial.printf("%d,%s,%02X:%02X:%02X:%02X:%02X:%02X,%d,%d\n",
        i + 1,
        strlen((char*)apList[i].ssid) ? (char*)apList[i].ssid : "<hidden>",
        apList[i].bssid[0], apList[i].bssid[1], apList[i].bssid[2],
        apList[i].bssid[3], apList[i].bssid[4], apList[i].bssid[5],
        apList[i].rssi,
        apList[i].primary
      );
    }

    // Export BLE devices
    Serial.printf("\nBLE Devices: %d\n", getActiveBLECount());
    for (int i = 0; i < bleDeviceCount; i++) {
      if (!bleDevices[i].isActive) continue;
      Serial.printf("%d,%s,%s,%d\n",
        i + 1,
        bleDevices[i].name.length() > 0 ? bleDevices[i].name.c_str() : "<unknown>",
        bleDevices[i].address.c_str(),
        bleDevices[i].rssi
      );
    }

    // Export security events
    Serial.printf("\nSecurity Events: %d\n", eventCount);
    for (int i = 0; i < eventCount; i++) {
      Serial.printf("%d,%d,%s\n",
        i + 1,
        eventLog[i].type,
        eventLog[i].message
      );
    }

    Serial.println("========== END EXPORT ==========\n");
  }

  drawExport();

  if (ev == BTN_BACK) {
    currentScreen = SCREEN_HISTORY_MENU;
    stopAllWifi();
    drawHistoryMenu();
    return;
  }
}

/* ========== Utility Handlers ========== */

void handleStats(ButtonEvent ev) {
  if (ev == BTN_SHORT) {
    currentScreen = SCREEN_MENU;
    drawMenu();
  }

  if (ev == BTN_LONG) {
    resetSession();
  }

  drawStats();
}
