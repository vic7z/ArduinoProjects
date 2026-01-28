/*
 * ESP32 Pocket RF Tool - Modular Version
 * ESP32-C3/S3 Zero + 0.96" OLED + RGB LED
 *
 * Hierarchical menu system for Wi-Fi & BLE analysis
 * All core functionality moved to separate .h/.cpp files
 *
 * Created from refactored codebase - Phase 1 Complete
 */

// Core Arduino libraries
#include <Arduino.h>

// Modular header files
#include "config.h"
#include "screens.h"
#include "screens_draw.h"
#include "screens_handlers.h"
#include "settings.h"
#include "wifi_scanner.h"
#include "ble_scanner.h"
#include "security.h"
#include "utils.h"
#include "menu.h"
#include "display.h"
#include "alerts.h"
#include "power.h"

/* ========== Global Hardware Objects ========== */
U8G2_SSD1306_128X64_NONAME_1_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE, 5, 4);
Adafruit_NeoPixel rgb(RGB_LED_COUNT, RGB_LED_PIN, NEO_GRB + NEO_KHZ800);
Preferences prefs;

/* ========== Global State ========== */
Settings settings = {1, -70, 50, true, false, 10, 60};  // scanSpeed, rssiThreshold, rgbBrightness, autoRefresh, csvLogging, deauthThreshold, screenTimeout
Screen currentScreen = SCREEN_MENU;

/* ========== Auto Mode State ========== */
uint8_t autoModeView = 0; // 0=summary, 1=top APs, 2=top BLE
uint32_t lastAutoWifiScan = 0;
uint32_t lastAutoBLEScan = 0;
uint16_t autoTotalAPs = 0;
uint16_t autoTotalBLE = 0;

/* ========== Insights State ========== */
uint8_t whySlowView = 0; // 0=analysis, 1=RSSI graph
RSSIHistory rssiHistory[MAX_TRACKED_APS];
uint32_t lastRSSISample = 0;

/* ========== Walk Test State ========== */
#define WALK_HISTORY_SIZE 60
int8_t walkRSSIHistory[WALK_HISTORY_SIZE];
uint8_t walkHistoryIndex = 0;
int8_t walkMinRSSI = 0;
int8_t walkMaxRSSI = -100;
int32_t walkRSSISum = 0;
uint16_t walkSampleCount = 0;
uint8_t walkTargetBSSID[6];  // Store BSSID instead of index
char walkTargetSSID[33];     // Store SSID for display
String walkTargetBLEAddr = "";
bool walkTestActive = false;
uint8_t walkTestView = 0; // 0=stats, 1=graph

/* ========== Event Log & Baseline ========== */
LogEvent eventLog[MAX_EVENTS];
uint8_t eventCount = 0;
uint8_t eventCursor = 0;
uint8_t eventScroll = 0;
Baseline baseline = {0, 0, 0, {0}, 0, false};
Baseline currentSnapshot = {0, 0, 0, {0}, 0, false};
uint32_t lastEnvCheck = 0;
uint32_t lastBaselineUpdate = 0;
bool prevAttackActive = false;
uint8_t prevRogueCount = 0;

/* ========== Device Monitor ========== */
MonitoredDevice monitoredDevices[MAX_MONITORED_DEVICES];
uint8_t monitoredDeviceCount = 0;
uint8_t deviceCursor = 0;
uint8_t deviceScroll = 0;
uint8_t deviceSelectedIndex = 0;

/* ========== Display Settings ========== */
uint8_t displaySettingCursor = 0; // 0=RGB Brightness, 1=Sleep Timeout

/* ========== Button State ========== */
// Action button state
bool lastReading = HIGH;
unsigned long lastDebounce = 0;
unsigned long pressStart = 0;

// Back button state
bool lastBackReading = HIGH;
unsigned long lastBackDebounce = 0;
bool backPressed = false;
unsigned long backPressStart = 0;
unsigned long lastBackFired = 0;

/* ========== Button Handler ========== */
ButtonEvent updateButton() {
  unsigned long now = millis();

  // Check BACK button first (dedicated button) - fire on release
  bool backBtn = digitalRead(BTN_BACK_PIN);
  if (backBtn != lastBackReading) {
    lastBackDebounce = now;
    lastBackReading = backBtn;
  }

  if (now - lastBackDebounce >= DEBOUNCE_MS) {
    if (backBtn == LOW && !backPressed) {
      // Button pressed down - mark as pressed and start timing
      backPressed = true;
      backPressStart = now;
    } else if (backBtn == HIGH && backPressed && (now - lastBackFired > 150)) {
      // Button released - check if long press (for sleep)
      unsigned long backHeld = now - backPressStart;
      backPressed = false;
      backPressStart = 0;
      lastBackFired = now;

      if (backHeld >= 1500) {
        // Long press BACK = Sleep
        Serial.println("[BTN] BACK LONG - SLEEP");
        return BTN_BACK_LONG;
      } else {
        // Normal BACK press
        Serial.println("[BTN] BACK");
        return BTN_BACK;
      }
    }
  }

  // Check ACTION button
  bool r = digitalRead(BTN_ACTION);
  if (r != lastReading) {
    lastDebounce = now;
    lastReading = r;
  }
  if (now - lastDebounce < DEBOUNCE_MS) return BTN_NONE;

  // Start press timing
  if (r == LOW && pressStart == 0) {
    pressStart = now;
  }

  // Button released - fire event based on hold duration
  if (r == HIGH && pressStart) {
    unsigned long held = now - pressStart;
    pressStart = 0;

    if (held >= LONG_PRESS_MS) {
      // Long hold (700ms) = Select/Enter
      Serial.printf("[BTN] LONG (held: %lums)\n", held);
      return BTN_LONG;
    }
    // Short press = Scroll/Navigate
    Serial.printf("[BTN] SHORT (held: %lums)\n", held);
    return BTN_SHORT;
  }

  return BTN_NONE;
}

/* ========== Setup ========== */
void setup() {
  Serial.begin(115200);
  pinMode(BTN_ACTION, INPUT_PULLUP);
  pinMode(BTN_BACK_PIN, INPUT_PULLUP);
  delay(100);

  // Load settings from NVS
  loadSettings();

  if (RGB_ENABLED) {
    rgb.begin();
    setRGB(RGB_WHITE);
  }

  oled.begin();
  oled.setContrast(128);

  // Show startup message
  oled.firstPage();
  do {
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(10, 25, "Wi-Fi Analyzer");
    oled.setFont(u8g2_font_5x7_tf);
    oled.drawStr(15, 40, "Initializing...");
  } while (oled.nextPage());

  delay(100);

  // Initialize Wi-Fi subsystem
  initWiFi();

  // Initialize BLE at startup (takes ~2 seconds)
  Serial.println("[INIT] Initializing BLE...");
  initBLE();
  Serial.println("[INIT] BLE ready");

  resetSession();
  drawMenu();

  if (RGB_ENABLED) setRGB(RGB_GREEN);

  Serial.println("ESP32-C3 Wi-Fi & BLE Analyzer Ready");
  Serial.println("Features: Wi-Fi 2.4GHz + BLE Scanner");
  Serial.println("CSV Format: timestamp,channel,rssi,type");
}

/* ========== Main Loop ========== */
void loop() {
  ButtonEvent ev = updateButton();

  // Long press BACK button to sleep immediately
  if (ev == BTN_BACK_LONG) {
    screenSleeping = true;
    oled.setPowerSave(1);  // Turn off OLED
    setRGB(RGB_OFF);       // Turn off LED
    stopAllWifi();         // Stop WiFi modem
    stopBLEScan();         // Stop BLE modem
    esp_wifi_stop();       // Completely stop WiFi for power savings
    Serial.println("[SLEEP] Manual sleep - modems off");
    return;
  }

  // Screen timeout management (battery saving)
  if (ev != BTN_NONE) {
    lastActivity = millis();
    if (screenSleeping) {
      // Wake up screen
      screenSleeping = false;
      oled.setPowerSave(0);  // Turn on OLED
      setRGB(RGB_GREEN);     // Turn on LED
      esp_wifi_start();      // Restart WiFi modem
      Serial.println("[WAKE] Screen on - modems active");
      return;  // Don't process this button press, just wake up
    }
  }

  // Don't sleep during active scanning screens
  bool activelyScanning = (currentScreen == SCREEN_AUTO_WATCH ||
                            currentScreen == SCREEN_MONITOR ||
                            currentScreen == SCREEN_ANALYZER ||
                            currentScreen == SCREEN_DEVICE_MONITOR ||
                            currentScreen == SCREEN_AP_LIST ||
                            currentScreen == SCREEN_BLE_SCAN ||
                            currentScreen == SCREEN_DEAUTH_WATCH ||
                            currentScreen == SCREEN_ROGUE_AP_WATCH ||
                            currentScreen == SCREEN_BLE_TRACKER_WATCH ||
                            currentScreen == SCREEN_WHY_IS_IT_SLOW ||
                            currentScreen == SCREEN_AP_WALK_TEST ||
                            currentScreen == SCREEN_BLE_WALK_TEST);

  if (settings.screenTimeout > 0 && !screenSleeping && !activelyScanning) {
    if (millis() - lastActivity > (settings.screenTimeout * 1000UL)) {
      screenSleeping = true;
      oled.setPowerSave(1);  // Turn off OLED
      setRGB(RGB_OFF);       // Turn off LED
      stopAllWifi();         // Stop WiFi modem
      stopBLEScan();         // Stop BLE modem
      esp_wifi_stop();       // Completely stop WiFi for power savings
      Serial.println("[SLEEP] Auto sleep - modems off");
      return;
    }
  }

  // Deauth detection (using configurable threshold)
  if (millis() - lastDeauthCheck > 1000) {
    deauthPerSecond = pktDeauth - lastDeauthCount;
    lastDeauthCount = pktDeauth;
    lastDeauthCheck = millis();

    if (deauthPerSecond > settings.deauthThreshold) {
      attackActive = true;
    } else if (deauthPerSecond == 0 && attackActive) {
      attackActive = false;
    }
  }

  // Signal alert
  signalAlert = (avgRssi > settings.rssiThreshold);

  updateRGBStatus();

  /* ----- MAIN MENU ----- */
  if (currentScreen == SCREEN_MENU) {
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
          stopAllWifi();
          lastScan = 0; // Force immediate scan
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
    delay(40);
    return;
  }


  /* ----- SECURITY MENU ----- */
  if (currentScreen == SCREEN_SECURITY_MENU) {
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
    delay(40);
    return;
  }

  /* ----- INSIGHTS MENU ----- */
  if (currentScreen == SCREEN_INSIGHTS_MENU) {
    handleInsightsMenu(ev);
    delay(40);
    return;
  }

  /* ----- HISTORY MENU ----- */
  if (currentScreen == SCREEN_HISTORY_MENU) {
    handleHistoryMenu(ev);
    delay(40);
    return;
  }

  /* ----- SYSTEM MENU ----- */
  if (currentScreen == SCREEN_SYSTEM_MENU) {
    handleSystemMenu(ev);
    delay(40);
    return;
  }

  /* ----- AUTO WATCH ----- */
  if (currentScreen == SCREEN_AUTO_WATCH) {
    handleAutoWatch(ev);
    delay(40);
    return;
  }


  /* ----- RF HEALTH ----- */
  if (currentScreen == SCREEN_RF_HEALTH) {
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
    delay(40);
    return;
  }

  /* ----- PLACEHOLDER SCREENS WITH BACK BUTTON ----- */
  /* Main menu direct screens */
  if (currentScreen == SCREEN_DEVICE_MONITOR) {
    handleDeviceMonitor(ev);
    delay(40);
    return;
  }

  /* Device Monitor detail */
  if (currentScreen == SCREEN_DEVICE_DETAIL) {
    handleDeviceDetail(ev);
    delay(40);
    return;
  }

  /* ----- AP WALK TEST ----- */
  if (currentScreen == SCREEN_AP_WALK_TEST) {
    handleAPWalkTest(ev);
    delay(40);
    return;
  }

  /* ----- BLE WALK TEST ----- */
  if (currentScreen == SCREEN_BLE_WALK_TEST) {
    handleBLEWalkTest(ev);
    delay(40);
    return;
  }

  /* Security submenu screens */
  if (currentScreen == SCREEN_BLE_TRACKER_WATCH) {
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
    delay(40);
    return;
  }

  /* ----- ALERT SETTINGS ----- */
  if (currentScreen == SCREEN_ALERT_SETTINGS) {
    static uint8_t settingIndex = 0;

    if (ev == BTN_SHORT) {
      // Navigate between settings
      settingIndex = (settingIndex + 1) % 2;
    } else if (ev == BTN_LONG) {
      // Adjust selected setting
      switch (settingIndex) {
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
    delay(40);
    return;
  }

  /* Insights submenu screens */
  if (currentScreen == SCREEN_WHY_IS_IT_SLOW) {
    handleWhyIsItSlow(ev);
    delay(40);
    return;
  }

  if (currentScreen == SCREEN_CHANNEL_RECOMMENDATION) {
    handleChannelRecommendation(ev);
    delay(40);
    return;
  }

  if (currentScreen == SCREEN_ENVIRONMENT_CHANGE) {
    handleEnvironmentChange(ev);
    delay(40);
    return;
  }

  if (currentScreen == SCREEN_QUICK_SNAPSHOT) {
    handleQuickSnapshot(ev);
    delay(40);
    return;
  }

  if (currentScreen == SCREEN_CHANNEL_SCORECARD) {
    handleChannelScorecard(ev);
    delay(40);
    return;
  }

  /* History submenu screens */
  if (currentScreen == SCREEN_EVENT_LOG) {
    handleEventLog(ev);
    delay(40);
    return;
  }

  if (currentScreen == SCREEN_BASELINE_COMPARE) {
    handleBaselineCompare(ev);
    delay(40);
    return;
  }

  if (currentScreen == SCREEN_EXPORT) {
    handleExport(ev);
    delay(40);
    return;
  }

  /* Display Settings - interactive */
  if (currentScreen == SCREEN_DISPLAY_SETTINGS) {
    // Adjust RGB brightness
    if (ev == BTN_SHORT) {
      settings.rgbBrightness = min(100, settings.rgbBrightness + 10);
      rgb.setBrightness((settings.rgbBrightness * 255) / 100);
      rgb.show();
    } else if (ev == BTN_LONG) {
      settings.rgbBrightness = max(0, settings.rgbBrightness - 10);
      rgb.setBrightness((settings.rgbBrightness * 255) / 100);
      rgb.show();
    }

    drawDisplaySettings();

    if (ev == BTN_BACK) {
      saveSettings();  // Save on exit
      currentScreen = SCREEN_SYSTEM_MENU;
      drawSystemMenu();
    }
    delay(40);
    return;
  }

  /* Radio Control - interactive */
  if (currentScreen == SCREEN_RADIO_CONTROL) {
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
    delay(40);
    return;
  }

  /* System submenu screens */
  if (currentScreen == SCREEN_BATTERY_POWER || currentScreen == SCREEN_ABOUT) {
    if (currentScreen == SCREEN_BATTERY_POWER) drawBatteryPower();
    else drawAbout();

    if (ev == BTN_BACK) {
      currentScreen = SCREEN_SYSTEM_MENU;
      drawSystemMenu();
    }
    delay(40);
    return;
  }

  if (currentScreen == SCREEN_POWER_MODE) {
    handlePowerMode(ev);
    delay(40);
    return;
  }

  /* ----- DEAUTH WATCH ----- */
  if (currentScreen == SCREEN_DEAUTH_WATCH) {
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
    delay(40);
    return;
  }

  /* ----- ROGUE AP WATCH ----- */
  if (currentScreen == SCREEN_ROGUE_AP_WATCH) {
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
    delay(40);
    return;
  }

  /* ----- AP LIST ----- */
  if (currentScreen == SCREEN_AP_LIST) {
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
    delay(40);
    return;
  }

  /* ----- AP DETAIL ----- */
  if (currentScreen == SCREEN_AP_DETAIL) {
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
    delay(40);
    return;
  }

  /* ----- AP COMPARE ----- */
  if (currentScreen == SCREEN_COMPARE) {
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
    delay(40);
    return;
  }

  /* ----- HIDDEN SSID ----- */
  if (currentScreen == SCREEN_HIDDEN_SSID) {
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
    delay(40);
    return;
  }


  /* ----- SESSION STATS ----- */
  if (currentScreen == SCREEN_STATS) {
    if (ev == BTN_SHORT) {
      currentScreen = SCREEN_MENU;
      drawMenu();
    }

    if (ev == BTN_LONG) {
      resetSession();
    }

    drawStats();
    delay(40);
    return;
  }



  /* ----- LIVE MONITOR ----- */
  if (currentScreen == SCREEN_MONITOR) {
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
    delay(40);
    return;
  }

  /* ----- CHANNEL ANALYZER ----- */
  if (currentScreen == SCREEN_ANALYZER) {
    handleAnalyzer(ev);
    delay(40);
    return;
  }

  /* ----- BLE SCANNER ----- */
  if (currentScreen == SCREEN_BLE_SCAN) {
    handleBLEScan(ev);
    delay(40);
    return;
  }

  /* ----- BLE DETAIL ----- */
  if (currentScreen == SCREEN_BLE_DETAIL) {
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
    delay(40);
    return;
  }
}
