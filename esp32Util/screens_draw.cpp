#include "screens_draw.h"
#include "wifi_scanner.h"
#include "ble_scanner.h"
#include "menu.h"
#include "display.h"
#include "utils.h"
#include "security.h"
#include "alerts.h"

/* ========== External Variables ========== */
// These are defined in the main .ino file
extern uint32_t pps, peak, peakPPS;
extern uint32_t history[HISTORY_SIZE];
extern uint8_t histIdx;
extern uint8_t currentChannel, selectedChannel, analyzerChannel;
extern float avgRssi;
extern bool frozen;
extern volatile uint32_t pktBeacon, pktData, pktDeauth, pktTotal;
extern uint32_t deauthPerSecond, totalDeauthDetected;
extern bool attackActive;
extern uint8_t apCursor, apScroll, apSelectedIndex;
extern bool apSortedOnce;
extern uint8_t apCompareA, apCompareB;
extern uint8_t bleCursor, bleScroll, bleSelectedIndex;
extern uint8_t hiddenCursor, hiddenScroll;
extern uint8_t settingsCursor;
extern bool loggingActive;
extern uint32_t loggedPackets;
extern uint32_t sessionStart;
extern uint32_t totalAPsFound;
extern uint32_t autoTotalAPs, autoTotalBLE;
extern uint8_t autoModeView;
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
extern uint8_t whySlowView;
extern RSSIHistory rssiHistory[MAX_TRACKED_APS];
extern uint8_t alertLevel;
extern uint32_t lastAlertBlink;
extern bool alertBlinkState;
extern bool prevAttackActive;
extern uint8_t prevRogueCount;
extern uint8_t deauthChannel;
extern uint8_t displaySettingCursor;

/* ========== Helper Functions ========== */

void drawGenericMenu(const char* title, const char** items, uint8_t itemCount, uint8_t& cursor) {
  oled.firstPage();
  do {
    oled.setFont(u8g2_font_6x10_tf);

    // Center title
    uint8_t titleWidth = strlen(title) * 6;
    uint8_t titleX = (128 - titleWidth) / 2;
    oled.drawStr(titleX, 8, title);

    uint8_t startIdx = 0;
    if (cursor >= 4) startIdx = cursor - 3;

    for (int i = 0; i < 4 && (startIdx + i) < itemCount; i++) {
      int itemIdx = startIdx + i;
      oled.drawStr(0, 22 + i * 12, itemIdx == cursor ? ">" : " ");
      oled.drawStr(10, 22 + i * 12, items[itemIdx]);
    }

    oled.setFont(u8g2_font_4x6_tf);
    if (startIdx > 0) oled.drawStr(122, 22, "^");
    if (startIdx + 4 < itemCount) oled.drawStr(122, 58, "v");
  } while (oled.nextPage());
}

void takeSnapshot(Baseline* snap) {
  snap->timestamp = millis();
  snap->totalAPs = apCount;

  // Calculate average RSSI
  int32_t rssiSum = 0;
  for (int i = 0; i < apCount; i++) {
    rssiSum += apList[i].rssi;
  }
  snap->avgRSSI = apCount > 0 ? rssiSum / apCount : 0;

  // Channel distribution
  memset(snap->channelDist, 0, sizeof(snap->channelDist));
  for (int i = 0; i < apCount; i++) {
    if (apList[i].primary >= 1 && apList[i].primary <= 13) {
      snap->channelDist[apList[i].primary - 1]++;
    }
  }

  snap->totalPackets = totalPackets;  // Use cumulative packet count
  snap->saved = true;
}

/* ========== Menu Drawing ========== */

void drawMenu() {
  drawGenericMenu("POCKET RF TOOL", mainMenuItems, MAIN_MENU_SIZE, mainMenuIndex);
}

void drawSecurityMenu() {
  drawGenericMenu("SECURITY", securityMenuItems, SECURITY_MENU_SIZE, securityMenuIndex);
}

void drawInsightsMenu() {
  drawGenericMenu("INSIGHTS", insightsMenuItems, INSIGHTS_MENU_SIZE, insightsMenuIndex);
}

void drawHistoryMenu() {
  drawGenericMenu("HISTORY", historyMenuItems, HISTORY_MENU_SIZE, historyMenuIndex);
}

void drawSystemMenu() {
  drawGenericMenu("SYSTEM", systemMenuItems, SYSTEM_MENU_SIZE, systemMenuIndex);
}

/* ========== Main Screens ========== */

void drawMonitor() {
  oled.firstPage();
  do {
    oled.drawFrame(0, 0, 128, 10);
    oled.setFont(u8g2_font_5x7_tf);
    oled.setCursor(2, 8);
    oled.printf("CH%02d %luP/s L:%d%%", currentChannel, pps, liveLoad());

    oled.setCursor(0, 18);
    oled.printf("B:%lu D:%lu X:%lu", pktBeacon, pktData, pktDeauth);

    int rssiBar = constrain((int)avgRssi + 100, 0, 50);
    oled.drawFrame(0, 20, 52, 6);
    oled.drawBox(1, 21, rssiBar, 4);
    oled.setCursor(54, 25);
    oled.printf("%ddBm", (int)avgRssi);

    // Signal alert indicator
    if (avgRssi > settings.rssiThreshold) {
      oled.setFont(u8g2_font_4x6_tf);
      oled.drawStr(110, 25, "!");
    }

    uint8_t graphY = 28;
    uint8_t graphH = 28;

    drawGrid(0, graphY, 128, graphH);

    for (int x = 0; x < 128; x++) {
      int idx = (histIdx + x) % HISTORY_SIZE;
      uint32_t val = history[idx];
      int barHeight = 0;
      if (peak > 0) barHeight = (val * graphH) / peak;

      if (barHeight > 0) {
        int solidHeight = barHeight / 3;
        if (solidHeight > 0) {
          oled.drawVLine(x, graphY + graphH - barHeight, solidHeight);
        }

        int midStart = graphY + graphH - barHeight + solidHeight;
        int midHeight = (barHeight * 2) / 5;
        for (int y = midStart; y < midStart + midHeight; y++) {
          if ((x + y) % 2 == 0) oled.drawPixel(x, y);
        }

        int bottomStart = midStart + midHeight;
        int bottomHeight = barHeight - solidHeight - midHeight;
        for (int y = bottomStart; y < bottomStart + bottomHeight; y++) {
          if ((x + y) % 3 == 0) oled.drawPixel(x, y);
        }
      }
    }

    oled.drawFrame(0, graphY, 128, graphH);

    oled.setFont(u8g2_font_5x7_tf);
    oled.setCursor(0, 63);
    oled.print(channelInsight());

    if (frozen) {
      oled.drawBox(118, 28, 9, 9);
      oled.setDrawColor(0);
      oled.drawStr(120, 35, "||");
      oled.setDrawColor(1);
    }

  } while (oled.nextPage());
}

void drawAnalyzer() {
  uint8_t selLoad = channelLoad(selectedChannel);
  uint8_t best = bestChannel();

  oled.firstPage();
  do {
    for (int ch = 1; ch <= MAX_CHANNEL; ch++) {
      int x = (ch - 1) * 9 + 2;
      uint8_t load = channelLoad(ch);
      int barH = min(load / 2, 35);

      if (barH > 0) {
        drawPatternBar(x, 45 - barH, 7, barH, load);
      }

      if (ch % 2 == 1) {
        oled.setFont(u8g2_font_4x6_tf);
        oled.setCursor(x, 52);
        oled.print(ch);
      }

      // Overlap indicator
      uint8_t overlap = countOverlappingAPs(ch);
      if (overlap > 2) {
        oled.setFont(u8g2_font_4x6_tf);
        oled.setCursor(x, 52);
        oled.print("!");
      }

      oled.setFont(u8g2_font_4x6_tf);
      if (ch == selectedChannel && ch == best) {
        oled.drawStr(x + 1, 6, "*v");
      } else if (ch == selectedChannel) {
        oled.drawStr(x + 2, 6, "v");
      } else if (ch == best) {
        oled.drawStr(x + 2, 6, "*");
      }
    }

    oled.setFont(u8g2_font_5x7_tf);
    oled.drawFrame(0, 54, 128, 10);
    oled.setCursor(2, 62);
    oled.printf("CH%02d:%s BEST:%02d", selectedChannel, loadQuality(selLoad), best);

  } while (oled.nextPage());
}

void drawAutoWatch() {
  oled.firstPage();
  do {
    oled.setFont(u8g2_font_6x10_tf);

    if (autoModeView == 0) {
      // Summary view
      oled.drawStr(28, 8, "AUTO WATCH");
      oled.setFont(u8g2_font_4x6_tf);
      oled.drawStr(8, 16, "Auto WiFi+BLE Monitor");

      oled.setFont(u8g2_font_5x7_tf);
      char buf[20];
      sprintf(buf, "APs:%d BLE:%d", autoTotalAPs, autoTotalBLE);
      oled.drawStr(0, 28, buf);

      sprintf(buf, "CH:%d PKT:%lu", currentChannel, pktTotal);
      oled.drawStr(0, 38, buf);

      if (deauthPerSecond > 0) {
        sprintf(buf, "DEAUTH:%lu/s", deauthPerSecond);
        oled.drawStr(0, 48, buf);
      } else {
        oled.drawStr(0, 48, "No attacks");
      }

    } else if (autoModeView == 1) {
      // Top APs view
      oled.drawStr(25, 8, "TOP APs");
      oled.setFont(u8g2_font_5x7_tf);

      uint8_t showCount = min(apCount, (uint16_t)4);
      for (int i = 0; i < showCount; i++) {
        char ssid[13];
        strncpy(ssid, (char*)apList[i].ssid, 12);
        ssid[12] = '\0';

        char buf[22];
        sprintf(buf, "%s %d", strlen((char*)apList[i].ssid) ? ssid : "<hidden>", apList[i].rssi);
        oled.drawStr(0, 20 + i * 10, buf);
      }

    } else if (autoModeView == 2) {
      // Top BLE view
      oled.drawStr(25, 8, "TOP BLE");
      oled.setFont(u8g2_font_5x7_tf);

      // Only show active devices
      uint8_t displayedCount = 0;
      for (int i = 0; i < bleDeviceCount && displayedCount < 4; i++) {
        if (!bleDevices[i].isActive) continue; // Skip inactive devices

        char name[13];
        if (bleDevices[i].name.length() > 0) {
          strncpy(name, bleDevices[i].name.c_str(), 12);
          name[12] = '\0';
        } else {
          strcpy(name, "Unknown");
        }

        char buf[22];
        sprintf(buf, "%s %d", name, bleDevices[i].rssi);
        oled.drawStr(0, 20 + displayedCount * 10, buf);
        displayedCount++;
      }

    } else if (autoModeView == 3) {
      // Channel APs view - show APs on current channel
      char title[20];
      sprintf(title, "CH%d APs", currentChannel);
      oled.drawStr(30, 8, title);
      oled.setFont(u8g2_font_4x6_tf);

      // Count and show APs on current channel
      uint8_t channelAPCount = 0;
      uint8_t shown = 0;
      for (int i = 0; i < apCount && shown < 5; i++) {
        if (apList[i].primary == currentChannel) {
          char ssid[11];
          strncpy(ssid, (char*)apList[i].ssid, 10);
          ssid[10] = '\0';

          char buf[24];
          sprintf(buf, "%s %d", strlen((char*)apList[i].ssid) ? ssid : "<hidden>", apList[i].rssi);
          oled.drawStr(0, 18 + shown * 9, buf);
          shown++;
          channelAPCount++;
        }
      }

      // Count remaining
      for (int i = 0; i < apCount; i++) {
        if (apList[i].primary == currentChannel) channelAPCount++;
      }

      oled.setFont(u8g2_font_4x6_tf);
      oled.setCursor(0, 63);
      oled.printf("Total: %d APs", channelAPCount);
    }

    oled.setFont(u8g2_font_4x6_tf);
    oled.drawStr(0, 63, "VIEW");
    oled.drawStr(90, 63, "BACK");

  } while (oled.nextPage());
}

void drawRFHealth() {
  // Calculate RF health metrics
  int totalDevices = apCount + bleDeviceCount;
  int avgRSSI = 0;
  int channelLoad[13] = {0}; // Channel congestion counter

  // Calculate average RSSI and channel load from APs
  if (apCount > 0) {
    long rssiSum = 0;
    for (int i = 0; i < apCount; i++) {
      rssiSum += apList[i].rssi;
      if (apList[i].primary >= 1 && apList[i].primary <= 13) {
        channelLoad[apList[i].primary - 1]++;
      }
    }
    avgRSSI = rssiSum / apCount;
  }

  // Find busiest channel
  int busiestCh = 0;
  int maxLoad = 0;
  for (int i = 0; i < 13; i++) {
    if (channelLoad[i] > maxLoad) {
      maxLoad = channelLoad[i];
      busiestCh = i + 1;
    }
  }

  // Calculate health score (0-100, higher is better)
  int healthScore = 100;
  if (maxLoad > 10) healthScore -= 30; // Very congested
  else if (maxLoad > 5) healthScore -= 15; // Moderately congested
  if (avgRSSI < -80) healthScore -= 20; // Weak signals
  else if (avgRSSI < -70) healthScore -= 10; // Fair signals
  if (totalDevices > 50) healthScore -= 20; // Very crowded
  else if (totalDevices > 30) healthScore -= 10; // Crowded

  healthScore = constrain(healthScore, 0, 100);

  oled.firstPage();
  do {
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(25, 10, "RF HEALTH");

    oled.setFont(u8g2_font_5x7_tf);

    // Device count
    oled.setCursor(0, 22);
    oled.printf("Devices: %d (%dAP+%dBLE)", totalDevices, apCount, bleDeviceCount);

    // Average RSSI
    oled.setCursor(0, 32);
    oled.printf("Avg RSSI: %ddBm", avgRSSI);

    // Busiest channel
    oled.setCursor(0, 42);
    oled.printf("Busy CH: %d (%d APs)", busiestCh, maxLoad);

    // Health score bar
    oled.setCursor(0, 52);
    oled.print("Health:");
    int barWidth = map(healthScore, 0, 100, 0, 75);
    oled.drawFrame(50, 48, 77, 8);
    oled.drawBox(51, 49, barWidth, 6);

    oled.setFont(u8g2_font_4x6_tf);
    oled.drawStr(85, 63, "BACK");
  } while (oled.nextPage());
}

void drawDeviceMonitor() {
  oled.firstPage();
  do {
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(10, 10, "DEVICE MONITOR");

    if (monitoredDeviceCount == 0) {
      oled.setFont(u8g2_font_5x7_tf);
      oled.setCursor(10, 30);
      oled.print("No devices yet");
      oled.setCursor(5, 42);
      oled.print("Scanning...");
    } else {
      oled.setFont(u8g2_font_4x6_tf);

      // Show up to 3 devices (with MAC addresses, need more space per device)
      uint8_t shown = 0;
      uint8_t activeIndex = 0;
      for (int i = 0; i < MAX_MONITORED_DEVICES && shown < 3; i++) {
        if (!monitoredDevices[i].active) continue;

        // Skip until we reach scroll position
        if (activeIndex < deviceScroll) {
          activeIndex++;
          continue;
        }

        uint8_t y = 16 + (shown * 16);

        // Highlight cursor
        if (shown == deviceCursor) {
          oled.drawBox(0, y - 6, 128, 15);
          oled.setDrawColor(0);
        }

        // Line 1: Type, Status, Name, RSSI
        oled.setCursor(0, y);
        oled.printf("%c%c", monitoredDevices[i].type == 0 ? 'W' : 'B',
                            monitoredDevices[i].isPresent ? '+' : '-');

        // Name (truncated to 10 chars)
        char name[11];
        strncpy(name, monitoredDevices[i].name, 10);
        name[10] = '\0';
        oled.setCursor(16, y);
        oled.print(name);

        oled.setCursor(104, y);
        oled.printf("%d", monitoredDevices[i].rssi);

        // Line 2: MAC address
        oled.setCursor(8, y + 7);
        if (monitoredDevices[i].type == 0) {
          // WiFi: show BSSID
          oled.printf("%02X:%02X:%02X:%02X:%02X:%02X",
            monitoredDevices[i].bssid[0], monitoredDevices[i].bssid[1],
            monitoredDevices[i].bssid[2], monitoredDevices[i].bssid[3],
            monitoredDevices[i].bssid[4], monitoredDevices[i].bssid[5]);
        } else {
          // BLE: show address
          oled.print(monitoredDevices[i].bleAddr);
        }

        if (shown == deviceCursor) oled.setDrawColor(1);

        shown++;
        activeIndex++;
      }

      // Scrollbar indicators
      if (deviceScroll > 0) oled.drawStr(122, 18, "^");
      if (activeIndex < monitoredDeviceCount) oled.drawStr(122, 58, "v");
    }

    // Footer
    oled.setFont(u8g2_font_4x6_tf);
    oled.drawLine(0, 54, 127, 54);
    oled.drawStr(0, 61, "SEL");
    oled.drawStr(30, 61, "DETAIL");
    oled.drawStr(85, 61, "BACK");
  } while (oled.nextPage());
}

void drawDeviceDetail() {
  if (deviceSelectedIndex >= MAX_MONITORED_DEVICES ||
      !monitoredDevices[deviceSelectedIndex].active) {
    drawPlaceholder("DEVICE DETAIL", "Invalid Device");
    return;
  }

  MonitoredDevice* dev = &monitoredDevices[deviceSelectedIndex];

  oled.firstPage();
  do {
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(10, 10, dev->type == 0 ? "WiFi DEVICE" : "BLE DEVICE");

    oled.setFont(u8g2_font_4x6_tf);

    // Device name
    oled.setCursor(0, 20);
    oled.print("Name:");
    oled.setCursor(30, 20);
    char name[20];
    strncpy(name, dev->name, 19);
    name[19] = '\0';
    oled.print(name);

    // BSSID/Address
    oled.setCursor(0, 28);
    if (dev->type == 0) {
      oled.print("BSSID:");
      oled.setCursor(30, 28);
      oled.printf("%02X:%02X:%02X:%02X:%02X:%02X",
        dev->bssid[0], dev->bssid[1], dev->bssid[2],
        dev->bssid[3], dev->bssid[4], dev->bssid[5]);
    } else {
      oled.print("Addr:");
      oled.setCursor(30, 28);
      oled.print(dev->bleAddr);
    }

    // RSSI
    oled.setCursor(0, 36);
    oled.printf("RSSI: %ddBm", dev->rssi);

    // Channel (WiFi only)
    if (dev->type == 0) {
      oled.setCursor(64, 36);
      oled.printf("CH: %d", dev->channel);
    }

    // Status
    oled.setCursor(0, 44);
    oled.print("Status:");
    oled.setCursor(35, 44);
    oled.print(dev->isPresent ? "Present" : "Not Seen");

    // Seen count
    oled.setCursor(0, 52);
    oled.printf("Seen: %d times", dev->seenCount);

    // Footer
    oled.drawLine(0, 54, 127, 54);
    oled.drawStr(85, 61, "BACK");
  } while (oled.nextPage());
}

/* ========== AP Scanner Screens ========== */

void drawApList() {
  oled.firstPage();
  do {
    oled.setFont(u8g2_font_5x7_tf);
    oled.drawFrame(0, 0, 128, 9);
    oled.setCursor(2, 7);
    oled.printf("APs:%d BEST:#%d", apCount, bestAPIndex() + 1);

    for (int row = 0; row < AP_VISIBLE; row++) {
      int idx = apScroll + row;
      if (idx >= apCount) break;

      int yPos = 18 + row * 15;

      if (row == apCursor) oled.drawStr(0, yPos, ">");

      oled.setFont(u8g2_font_4x6_tf);
      if (apList[idx].authmode == WIFI_AUTH_OPEN) {
        oled.drawStr(7, yPos, "!");
      } else if (apList[idx].authmode >= WIFI_AUTH_WPA2_PSK) {
        oled.drawStr(7, yPos, "*");
      } else {
        oled.drawStr(7, yPos, "~");
      }

      oled.setFont(u8g2_font_5x7_tf);
      char ssidBuf[11] = {0};
      const char* ssid = strlen((char*)apList[idx].ssid) ? (char*)apList[idx].ssid : "<hidden>";
      strncpy(ssidBuf, ssid, 10);
      ssidBuf[10] = 0;
      oled.drawStr(13, yPos, ssidBuf);

      oled.setFont(u8g2_font_4x6_tf);
      oled.setCursor(75, yPos);
      oled.printf("CH%d", apList[idx].primary);

      // Quality grade
      char grade = getQualityGrade(&apList[idx]);
      oled.setCursor(100, yPos);
      oled.print(grade);

      // Signal bars
      int rssi = apList[idx].rssi;
      int bars = 0;
      if (rssi >= -50) bars = 4;
      else if (rssi >= -60) bars = 3;
      else if (rssi >= -70) bars = 2;
      else if (rssi >= -80) bars = 1;

      int barX = 110;
      for (int b = 0; b < 4; b++) {
        int barH = 2 + b * 2;
        if (b < bars) {
          oled.drawBox(barX + b * 4, yPos - barH, 2, barH);
        } else {
          oled.drawFrame(barX + b * 4, yPos - barH, 2, barH);
        }
      }

      // Vendor/Distance on second line
      oled.setFont(u8g2_font_4x6_tf);
      oled.setCursor(13, yPos + 7);
      oled.printf("%s %.1fm", getVendor(apList[idx].bssid),
                  estimateDistance(apList[idx].rssi));
    }

    oled.setFont(u8g2_font_4x6_tf);
    if (apScroll > 0) oled.drawStr(122, 18, "^");
    if (apScroll + AP_VISIBLE < apCount) oled.drawStr(122, 61, "v");

  } while (oled.nextPage());
}

void drawApDetail() {
  wifi_ap_record_t* ap = &apList[apSelectedIndex];
  oled.firstPage();
  do {
    oled.setFont(u8g2_font_5x7_tf);
    oled.setCursor(0, 7);

    char ssidBuf[17] = {0};
    strncpy(ssidBuf, (char*)ap->ssid, 16);
    oled.printf("SSID:%s", strlen((char*)ap->ssid) ? ssidBuf : "<hidden>");

    oled.setCursor(0, 16);
    oled.printf("CH:%d RSSI:%d (%c)", ap->primary, ap->rssi, getQualityGrade(ap));

    oled.setCursor(0, 25);
    oled.printf("SEC:%s", authStr(ap->authmode));

    oled.setCursor(0, 34);
    oled.printf("Vendor:%s", getVendor(ap->bssid));

    oled.setCursor(0, 43);
    oled.printf("Distance:%.1fm", estimateDistance(ap->rssi));

    oled.setFont(u8g2_font_4x6_tf);
    oled.setCursor(0, 51);
    oled.printf("MAC:%02X:%02X:%02X:%02X:%02X:%02X",
      ap->bssid[0], ap->bssid[1], ap->bssid[2],
      ap->bssid[3], ap->bssid[4], ap->bssid[5]);

    // Button hints
    oled.drawLine(0, 54, 127, 54);  // Separator line
    oled.setFont(u8g2_font_4x6_tf);
    oled.drawStr(0, 61, "LONG=Walk BACK=List");
  } while (oled.nextPage());
}

void drawAPWalkTest() {
  if (walkTestView == 0) {
    // Stats view
    oled.firstPage();
    do {
      oled.setFont(u8g2_font_6x10_tf);
      oled.drawStr(15, 10, "AP WALK TEST");

      if (walkTestActive) {
        // Show SSID (from saved name)
        oled.setFont(u8g2_font_5x7_tf);
        char ssid[17];
        strncpy(ssid, walkTargetSSID, 16);
        ssid[16] = '\0';
        oled.setCursor(0, 20);
        oled.printf("%s", strlen(walkTargetSSID) ? ssid : "<hidden>");

        // Current RSSI (from last sample)
        int8_t currentRSSI = (walkSampleCount > 0) ? walkRSSIHistory[(walkHistoryIndex - 1 + WALK_HISTORY_SIZE) % WALK_HISTORY_SIZE] : 0;
        oled.setCursor(0, 30);
        if (currentRSSI != 0) {
          oled.printf("Now: %ddBm", currentRSSI);
        } else {
          oled.print("Scanning...");
        }

        // Min/Max/Avg
        int8_t avgRSSI = walkSampleCount > 0 ? (walkRSSISum / walkSampleCount) : 0;
        oled.setFont(u8g2_font_4x6_tf);
        oled.setCursor(0, 40);
        oled.printf("Min:%d Max:%d Avg:%d", walkMinRSSI, walkMaxRSSI, avgRSSI);

        // Draw RSSI history graph
        oled.drawFrame(0, 45, 128, 16);
        for (int i = 0; i < WALK_HISTORY_SIZE && i < 126; i++) {
          int idx = (walkHistoryIndex + i) % WALK_HISTORY_SIZE;
          if (walkRSSIHistory[idx] != 0) {
            int height = map(constrain(walkRSSIHistory[idx], -90, -30), -90, -30, 1, 14);
            int x = 1 + (i * 126 / WALK_HISTORY_SIZE);
            oled.drawVLine(x, 60 - height, height);
          }
        }

        oled.setFont(u8g2_font_4x6_tf);
        oled.drawStr(50, 64, "SHORT=Graph");
      } else {
        oled.setFont(u8g2_font_5x7_tf);
        oled.drawStr(10, 35, "No AP selected");
      }
    } while (oled.nextPage());
  } else {
    // Full graph view
    oled.firstPage();
    do {
      oled.setFont(u8g2_font_6x10_tf);
      oled.drawStr(20, 10, "RSSI Graph");

      // Draw graph area
      const uint8_t graphX = 10;
      const uint8_t graphY = 15;
      const uint8_t graphW = 108;
      const uint8_t graphH = 35;

      // Draw graph border
      oled.drawFrame(graphX, graphY, graphW, graphH);

      // Draw RSSI scale
      oled.setFont(u8g2_font_4x6_tf);
      oled.drawStr(0, graphY + 5, "-30");
      oled.drawStr(0, graphY + graphH - 2, "-90");

      // Draw horizontal grid lines
      for (int i = 1; i < 3; i++) {
        uint8_t y = graphY + (graphH * i / 3);
        for (uint8_t x = graphX; x < graphX + graphW; x += 4) {
          oled.drawPixel(x, y);
        }
      }

      // Plot RSSI history
      for (int i = 1; i < WALK_HISTORY_SIZE; i++) {
        int idx1 = (walkHistoryIndex + i - 1) % WALK_HISTORY_SIZE;
        int idx2 = (walkHistoryIndex + i) % WALK_HISTORY_SIZE;
        int8_t rssi1 = walkRSSIHistory[idx1];
        int8_t rssi2 = walkRSSIHistory[idx2];

        if (rssi1 != 0 && rssi2 != 0) {
          // Map RSSI (-30 to -90) to graph Y coordinates
          uint8_t y1 = graphY + graphH - ((rssi1 + 90) * graphH / 60);
          uint8_t y2 = graphY + graphH - ((rssi2 + 90) * graphH / 60);

          // Map sample index to X coordinates
          uint8_t x1 = graphX + (i - 1) * graphW / WALK_HISTORY_SIZE;
          uint8_t x2 = graphX + i * graphW / WALK_HISTORY_SIZE;

          // Draw line between points
          oled.drawLine(x1, y1, x2, y2);
        }
      }

      // Show AP name and stats
      oled.setFont(u8g2_font_4x6_tf);
      oled.setCursor(0, 54);
      char ssid[17];
      strncpy(ssid, walkTargetSSID, 16);
      ssid[16] = '\0';
      oled.printf("%s", strlen(walkTargetSSID) ? ssid : "<hidden>");

      int8_t avgRSSI = walkSampleCount > 0 ? (walkRSSISum / walkSampleCount) : 0;
      oled.setCursor(0, 60);
      oled.printf("Avg:%d Min:%d Max:%d", avgRSSI, walkMinRSSI, walkMaxRSSI);
      oled.drawStr(60, 64, "SHORT=Stats");
    } while (oled.nextPage());
  }
}

void drawCompare() {
  wifi_ap_record_t* apA = &apList[apCompareA];
  wifi_ap_record_t* apB = &apList[apCompareB];

  oled.firstPage();
  do {
    oled.setFont(u8g2_font_5x7_tf);
    oled.drawStr(30, 7, "AP COMPARE");

    oled.setFont(u8g2_font_4x6_tf);

    // AP A (left)
    oled.setCursor(0, 16);
    char ssidA[9] = {0};
    strncpy(ssidA, (char*)apA->ssid, 8);
    oled.printf("A:%s", ssidA);

    oled.setCursor(0, 24);
    oled.printf("RSSI:%d", apA->rssi);

    oled.setCursor(0, 32);
    oled.printf("CH:%d", apA->primary);

    oled.setCursor(0, 40);
    oled.printf("SEC:%s", apA->authmode >= WIFI_AUTH_WPA2_PSK ? "Good" : "Weak");

    oled.setCursor(0, 48);
    oled.printf("GRADE:%c", getQualityGrade(apA));

    oled.setCursor(0, 56);
    oled.printf("DIST:%.1fm", estimateDistance(apA->rssi));

    // Divider
    for (int y = 12; y < 60; y += 2) {
      oled.drawPixel(64, y);
    }

    // AP B (right)
    oled.setCursor(68, 16);
    char ssidB[9] = {0};
    strncpy(ssidB, (char*)apB->ssid, 8);
    oled.printf("B:%s", ssidB);

    oled.setCursor(68, 24);
    oled.printf("RSSI:%d", apB->rssi);

    oled.setCursor(68, 32);
    oled.printf("CH:%d", apB->primary);

    oled.setCursor(68, 40);
    oled.printf("SEC:%s", apB->authmode >= WIFI_AUTH_WPA2_PSK ? "Good" : "Weak");

    oled.setCursor(68, 48);
    oled.printf("GRADE:%c", getQualityGrade(apB));

    oled.setCursor(68, 56);
    oled.printf("DIST:%.1fm", estimateDistance(apB->rssi));

    // Winner indicator
    char gradeA = getQualityGrade(apA);
    char gradeB = getQualityGrade(apB);
    if (gradeA < gradeB) {
      oled.drawStr(0, 63, "WINNER");
    } else if (gradeB < gradeA) {
      oled.drawStr(90, 63, "WINNER");
    } else {
      oled.drawStr(48, 63, "TIE");
    }

  } while (oled.nextPage());
}

/* ========== BLE Scanner Screens ========== */

void drawBLEScan() {
  oled.firstPage();
  do {
    oled.setFont(u8g2_font_5x7_tf);
    oled.drawFrame(0, 0, 128, 9);
    oled.setCursor(2, 7);

    uint8_t activeCount = 0;
    for (int i = 0; i < bleDeviceCount; i++) {
      if (bleDevices[i].isActive) activeCount++;
    }
    oled.printf("BLE:%d (%d)", bleDeviceCount, activeCount);

    if (bleDeviceCount == 0) {
      oled.setFont(u8g2_font_6x10_tf);
      oled.drawStr(20, 35, "Scanning...");
    } else {
      for (int row = 0; row < BLE_VISIBLE; row++) {
        int idx = bleScroll + row;
        if (idx >= bleDeviceCount) break;

        int yPos = 18 + row * 15;

        if (row == bleCursor) oled.drawStr(0, yPos, ">");

        // Beacon type indicator
        oled.setFont(u8g2_font_4x6_tf);
        if (bleDevices[idx].advType == 1) {
          oled.drawStr(7, yPos, "B"); // Beacon
        } else if (bleDevices[idx].isActive) {
          oled.drawStr(7, yPos, "*"); // Active
        } else {
          oled.drawStr(7, yPos, "-"); // Inactive
        }

        // Device name (truncated)
        oled.setFont(u8g2_font_5x7_tf);
        char nameBuf[11] = {0};
        strncpy(nameBuf, bleDevices[idx].name.c_str(), 10);
        nameBuf[10] = 0;
        oled.drawStr(13, yPos, nameBuf);

        // RSSI
        oled.setFont(u8g2_font_4x6_tf);
        oled.setCursor(80, yPos);
        oled.printf("%d", bleDevices[idx].rssi);

        // Signal bars
        int rssi = bleDevices[idx].rssi;
        int bars = 0;
        if (rssi >= -50) bars = 4;
        else if (rssi >= -65) bars = 3;
        else if (rssi >= -75) bars = 2;
        else if (rssi >= -85) bars = 1;

        int barX = 106;
        for (int b = 0; b < 4; b++) {
          int barH = 2 + b * 2;
          if (b < bars) {
            oled.drawBox(barX + b * 4, yPos - barH, 2, barH);
          } else {
            oled.drawFrame(barX + b * 4, yPos - barH, 2, barH);
          }
        }

        // MAC Address on second line
        oled.setFont(u8g2_font_4x6_tf);
        oled.setCursor(13, yPos + 7);
        char macBuf[13] = {0};
        strncpy(macBuf, bleDevices[idx].address.c_str(), 12);
        oled.print(macBuf);
      }

      oled.setFont(u8g2_font_4x6_tf);
      if (bleScroll > 0) oled.drawStr(122, 18, "^");
      if (bleScroll + BLE_VISIBLE < bleDeviceCount) oled.drawStr(122, 61, "v");
    }

  } while (oled.nextPage());
}

void drawBLEDetail() {
  BLEDeviceInfo* dev = &bleDevices[bleSelectedIndex];

  oled.firstPage();
  do {
    oled.setFont(u8g2_font_5x7_tf);
    oled.setCursor(0, 7);

    char nameBuf[17] = {0};
    strncpy(nameBuf, dev->name.c_str(), 16);
    oled.printf("Name:%s", nameBuf);

    oled.setCursor(0, 16);
    oled.printf("RSSI:%d dBm", dev->rssi);

    oled.setCursor(0, 25);
    float dist = estimateDistance(dev->rssi); // Reuse Wi-Fi formula
    oled.printf("Distance:~%.1fm", dist);

    oled.setCursor(0, 34);
    const char* typeStr = "Unknown";
    if (dev->advType == 1) typeStr = "iBeacon";
    else if (dev->advType == 2) typeStr = "Eddystone";
    oled.printf("Type:%s", typeStr);

    oled.setCursor(0, 43);
    oled.printf("Status:%s", dev->isActive ? "Active" : "Lost");

    oled.setFont(u8g2_font_4x6_tf);
    oled.setCursor(0, 51);
    oled.printf("MAC:%s", dev->address.c_str());

    // Button hints
    oled.drawLine(0, 54, 127, 54);  // Separator line
    oled.setFont(u8g2_font_4x6_tf);
    oled.drawStr(0, 61, "LONG=Walk BACK=List");
  } while (oled.nextPage());
}

void drawBLEWalkTest() {
  if (walkTestView == 0) {
    // Stats view
    oled.firstPage();
    do {
      oled.setFont(u8g2_font_6x10_tf);
      oled.drawStr(10, 10, "BLE WALK TEST");

      if (walkTestActive && walkTargetBLEAddr.length() > 0) {
        // Show address (truncated, from saved data)
        oled.setFont(u8g2_font_5x7_tf);
        char addr[17];
        strncpy(addr, walkTargetBLEAddr.c_str(), 16);
        addr[16] = '\0';
        oled.setCursor(0, 20);
        oled.print(addr);

        // Current RSSI (from last sample in history)
        int8_t currentRSSI = walkRSSIHistory[(walkHistoryIndex - 1 + WALK_HISTORY_SIZE) % WALK_HISTORY_SIZE];
        if (currentRSSI == 0 && walkSampleCount > 0) {
          // Find last non-zero sample
          for (int i = 1; i < WALK_HISTORY_SIZE; i++) {
            int idx = (walkHistoryIndex - i + WALK_HISTORY_SIZE) % WALK_HISTORY_SIZE;
            if (walkRSSIHistory[idx] != 0) {
              currentRSSI = walkRSSIHistory[idx];
              break;
            }
          }
        }

        oled.setCursor(0, 30);
        oled.printf("Now: %ddBm", currentRSSI);

        // Min/Max/Avg
        int8_t avgRSSI = walkSampleCount > 0 ? (walkRSSISum / walkSampleCount) : currentRSSI;
        oled.setFont(u8g2_font_4x6_tf);
        oled.setCursor(0, 40);
        oled.printf("Min:%d Max:%d Avg:%d", walkMinRSSI, walkMaxRSSI, avgRSSI);

        // Draw RSSI history graph
        oled.drawFrame(0, 45, 128, 16);
        for (int i = 0; i < WALK_HISTORY_SIZE && i < 126; i++) {
          int idx = (walkHistoryIndex + i) % WALK_HISTORY_SIZE;
          if (walkRSSIHistory[idx] != 0) {
            int height = map(constrain(walkRSSIHistory[idx], -90, -30), -90, -30, 1, 14);
            int x = 1 + (i * 126 / WALK_HISTORY_SIZE);
            oled.drawVLine(x, 60 - height, height);
          }
        }

        oled.setFont(u8g2_font_4x6_tf);
        oled.drawStr(50, 64, "SHORT=Graph");
      } else {
        oled.setFont(u8g2_font_5x7_tf);
        oled.drawStr(5, 35, "No BLE device selected");
      }
    } while (oled.nextPage());
  } else {
    // Full graph view
    oled.firstPage();
    do {
      oled.setFont(u8g2_font_6x10_tf);
      oled.drawStr(20, 10, "RSSI Graph");

      // Draw graph area
      const uint8_t graphX = 10;
      const uint8_t graphY = 15;
      const uint8_t graphW = 108;
      const uint8_t graphH = 35;

      // Draw graph border
      oled.drawFrame(graphX, graphY, graphW, graphH);

      // Draw RSSI scale
      oled.setFont(u8g2_font_4x6_tf);
      oled.drawStr(0, graphY + 5, "-30");
      oled.drawStr(0, graphY + graphH - 2, "-90");

      // Draw horizontal grid lines
      for (int i = 1; i < 3; i++) {
        uint8_t y = graphY + (graphH * i / 3);
        for (uint8_t x = graphX; x < graphX + graphW; x += 4) {
          oled.drawPixel(x, y);
        }
      }

      // Plot RSSI history
      for (int i = 1; i < WALK_HISTORY_SIZE; i++) {
        int idx1 = (walkHistoryIndex + i - 1) % WALK_HISTORY_SIZE;
        int idx2 = (walkHistoryIndex + i) % WALK_HISTORY_SIZE;
        int8_t rssi1 = walkRSSIHistory[idx1];
        int8_t rssi2 = walkRSSIHistory[idx2];

        if (rssi1 != 0 && rssi2 != 0) {
          // Map RSSI (-30 to -90) to graph Y coordinates
          uint8_t y1 = graphY + graphH - ((rssi1 + 90) * graphH / 60);
          uint8_t y2 = graphY + graphH - ((rssi2 + 90) * graphH / 60);

          // Map sample index to X coordinates
          uint8_t x1 = graphX + (i - 1) * graphW / WALK_HISTORY_SIZE;
          uint8_t x2 = graphX + i * graphW / WALK_HISTORY_SIZE;

          // Draw line between points
          oled.drawLine(x1, y1, x2, y2);
        }
      }

      // Show BLE address and stats
      oled.setFont(u8g2_font_4x6_tf);
      oled.setCursor(0, 54);
      char addr[17];
      strncpy(addr, walkTargetBLEAddr.c_str(), 16);
      addr[16] = '\0';
      oled.printf("%s", addr);

      int8_t avgRSSI = walkSampleCount > 0 ? (walkRSSISum / walkSampleCount) : 0;
      oled.setCursor(0, 60);
      oled.printf("Avg:%d Min:%d Max:%d", avgRSSI, walkMinRSSI, walkMaxRSSI);
      oled.drawStr(60, 64, "SHORT=Stats");
    } while (oled.nextPage());
  }
}

/* ========== Security Screens ========== */

void drawDeauthWatch() {
  oled.firstPage();
  do {
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(10, 10, "DEAUTH WATCH");

    oled.setFont(u8g2_font_5x7_tf);
    oled.setCursor(0, 22);
    oled.printf("Channel: %d", currentChannel);

    oled.setCursor(0, 32);
    oled.printf("Rate: %lu/sec", deauthPerSecond);

    oled.setCursor(0, 42);
    oled.printf("Total: %lu", totalDeauthDetected);

    // Status with visual indicator
    oled.setCursor(0, 52);
    if (attackActive) {
      oled.setFont(u8g2_font_6x10_tf);
      oled.print("!! ATTACK !!");
    } else {
      oled.print("Status: Normal");
    }

    // Footer
    oled.drawLine(0, 54, 127, 54);
    oled.setFont(u8g2_font_4x6_tf);
    oled.drawStr(30, 61, "BACK=Menu");
  } while (oled.nextPage());
}

void drawRogueAPWatch() {
  oled.firstPage();
  do {
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(5, 10, "ROGUE AP WATCH");

    oled.setFont(u8g2_font_5x7_tf);
    oled.setCursor(0, 22);
    oled.printf("Scanning: %d APs", apCount);

    oled.setCursor(0, 32);
    if (rogueCount > 0) {
      oled.printf("!! %d Rogue(s) !!", rogueCount);
    } else {
      oled.print("Status: Clean");
    }

    // List rogue APs if found
    if (rogueCount > 0) {
      oled.setFont(u8g2_font_4x6_tf);
      oled.setCursor(0, 42);
      oled.print("Evil Twin Detected:");
      for (int i = 0; i < min((int)rogueCount, 2); i++) {
        oled.setCursor(5, 50 + i * 8);
        char ssid[17];
        strncpy(ssid, rogueList[i].ssid, 16);
        ssid[16] = '\0';
        oled.printf("%s", ssid);
      }
    }

    // Footer
    oled.drawLine(0, 54, 127, 54);
    oled.setFont(u8g2_font_4x6_tf);
    oled.drawStr(30, 61, "BACK=Menu");
  } while (oled.nextPage());
}

void drawBLETrackerWatch() {
  // Count potential trackers (iBeacon, Eddystone, or devices with no name)
  int trackerCount = 0;
  for (int i = 0; i < bleDeviceCount; i++) {
    if (bleDevices[i].advType > 0 || !bleDevices[i].hasName) {
      trackerCount++;
    }
  }

  oled.firstPage();
  do {
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(5, 10, "BLE TRACKER");

    oled.setFont(u8g2_font_5x7_tf);
    oled.setCursor(0, 22);
    oled.printf("Total BLE: %d", bleDeviceCount);

    oled.setCursor(0, 32);
    if (trackerCount > 0) {
      oled.printf("Trackers: %d", trackerCount);
    } else {
      oled.print("No trackers found");
    }

    // List potential trackers
    if (trackerCount > 0) {
      oled.setFont(u8g2_font_4x6_tf);
      oled.setCursor(0, 42);
      oled.print("Detected:");

      int shown = 0;
      for (int i = 0; i < bleDeviceCount && shown < 2; i++) {
        if (bleDevices[i].advType > 0 || !bleDevices[i].hasName) {
          oled.setCursor(5, 50 + shown * 8);
          char addr[13];
          strncpy(addr, bleDevices[i].address.c_str(), 12);
          addr[12] = '\0';
          oled.printf("%s %ddBm", addr, bleDevices[i].rssi);
          shown++;
        }
      }
    }

    // Footer
    oled.drawLine(0, 54, 127, 54);
    oled.setFont(u8g2_font_4x6_tf);
    oled.drawStr(30, 61, "BACK=Menu");
  } while (oled.nextPage());
}

void drawAlertSettings() {
  oled.firstPage();
  do {
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(5, 10, "ALERT SETTINGS");

    oled.setFont(u8g2_font_5x7_tf);

    // Deauth Threshold
    oled.setCursor(0, 22);
    if (alertSettingIndex == 0) oled.print(">");
    oled.setCursor(10, 22);
    oled.printf("Deauth: %d/sec", settings.deauthThreshold);

    // Screen Timeout
    oled.setCursor(0, 32);
    if (alertSettingIndex == 1) oled.print(">");
    oled.setCursor(10, 32);
    if (settings.screenTimeout == 0) {
      oled.print("Timeout: Never");
    } else {
      oled.printf("Timeout: %ds", settings.screenTimeout);
    }

    // Instructions
    oled.setFont(u8g2_font_4x6_tf);
    oled.setCursor(0, 46);
    oled.print("SHORT=Next LONG=Adjust");

    // Footer
    oled.drawLine(0, 54, 127, 54);
    oled.drawStr(30, 61, "BACK=Save");
  } while (oled.nextPage());
}

/* ========== Insights Screens ========== */

void drawWhyIsItSlow() {
  if (whySlowView == 0) {
    // Analysis view
    int channelLoad[13] = {0};
    int maxLoad = 0;
    int avgRSSITotal = 0;
    int weakSignalCount = 0;

    // Count APs per channel and analyze signals
    for (int i = 0; i < apCount; i++) {
      if (apList[i].primary >= 1 && apList[i].primary <= 13) {
        channelLoad[apList[i].primary - 1]++;
      }
      avgRSSITotal += apList[i].rssi;
      if (apList[i].rssi < -75) weakSignalCount++;
    }

    // Find max channel load
    for (int i = 0; i < 13; i++) {
      if (channelLoad[i] > maxLoad) maxLoad = channelLoad[i];
    }

    int avgRSSI = apCount > 0 ? avgRSSITotal / apCount : -100;

    oled.firstPage();
    do {
      oled.setFont(u8g2_font_6x10_tf);
      oled.drawStr(5, 10, "WHY IS IT SLOW?");

      oled.setFont(u8g2_font_4x6_tf);

      // Issue 1: Channel congestion
      oled.setCursor(0, 20);
      if (maxLoad > 10) {
        oled.print("! Congested channel");
      } else if (maxLoad > 5) {
        oled.print("  Moderate congestion");
      } else {
        oled.print("  Channel load OK");
      }

      // Issue 2: Signal strength
      oled.setCursor(0, 28);
      if (avgRSSI < -80) {
        oled.print("! Weak signals");
      } else if (avgRSSI < -70) {
        oled.print("  Fair signal strength");
      } else {
        oled.print("  Signal strength OK");
      }

      // Issue 3: Interference
      oled.setCursor(0, 36);
      if (deauthPerSecond > 5) {
        oled.print("! High interference");
      } else {
        oled.print("  Interference OK");
      }

      // Issue 4: Too many devices
      oled.setCursor(0, 44);
      if (apCount > 30) {
        oled.print("! Too many APs nearby");
      } else {
        oled.print("  AP count OK");
      }

      // Summary
      oled.setFont(u8g2_font_5x7_tf);
      oled.setCursor(0, 53);
      int issues = (maxLoad > 10 ? 1 : 0) + (avgRSSI < -80 ? 1 : 0) +
                   (deauthPerSecond > 5 ? 1 : 0) + (apCount > 30 ? 1 : 0);
      oled.printf("%d issue(s) found", issues);

      // Footer
      oled.drawLine(0, 54, 127, 54);
      oled.setFont(u8g2_font_4x6_tf);
      oled.drawStr(10, 61, "LONG=Graph BACK=Menu");
    } while (oled.nextPage());
  } else {
    // RSSI Graph view
    oled.firstPage();
    do {
      oled.setFont(u8g2_font_6x10_tf);
      oled.drawStr(10, 10, "RSSI Over Time");

      // Draw graph area
      const uint8_t graphX = 10;
      const uint8_t graphY = 15;
      const uint8_t graphW = 108;
      const uint8_t graphH = 35;

      // Draw graph border
      oled.drawFrame(graphX, graphY, graphW, graphH);

      // Draw RSSI scale (-40 to -100)
      oled.setFont(u8g2_font_4x6_tf);
      oled.drawStr(0, graphY + 5, "-40");
      oled.drawStr(0, graphY + graphH - 2, "-100");

      // Draw horizontal grid lines
      for (int i = 1; i < 3; i++) {
        uint8_t y = graphY + (graphH * i / 3);
        for (uint8_t x = graphX; x < graphX + graphW; x += 4) {
          oled.drawPixel(x, y);
        }
      }

      // Plot RSSI history for each tracked AP
      for (int apIdx = 0; apIdx < MAX_TRACKED_APS; apIdx++) {
        if (!rssiHistory[apIdx].active) continue;

        // Draw line graph
        for (int i = 1; i < RSSI_HISTORY_SIZE; i++) {
          int8_t rssi1 = rssiHistory[apIdx].rssiSamples[i - 1];
          int8_t rssi2 = rssiHistory[apIdx].rssiSamples[i];

          if (rssi1 < -100 || rssi2 < -100) continue; // Skip invalid samples

          // Map RSSI (-40 to -100) to graph Y coordinates
          uint8_t y1 = graphY + graphH - ((rssi1 + 100) * graphH / 60);
          uint8_t y2 = graphY + graphH - ((rssi2 + 100) * graphH / 60);

          // Map sample index to X coordinates
          uint8_t x1 = graphX + (i - 1) * graphW / RSSI_HISTORY_SIZE;
          uint8_t x2 = graphX + i * graphW / RSSI_HISTORY_SIZE;

          // Draw line between points
          oled.drawLine(x1, y1, x2, y2);
        }
      }

      // Show tracked AP names (all 3 in single line)
      oled.setFont(u8g2_font_4x6_tf);
      char apNames[40] = "";
      for (int i = 0; i < MAX_TRACKED_APS; i++) {
        if (rssiHistory[i].active) {
          // Find the AP by BSSID
          for (int j = 0; j < apCount; j++) {
            if (memcmp(rssiHistory[i].bssid, apList[j].bssid, 6) == 0) {
              // Truncate SSID to fit - show only first 5 chars
              char truncSSID[7];
              strncpy(truncSSID, (char*)apList[j].ssid, 5);
              truncSSID[5] = '\0';
              if (strlen(apNames) > 0) strcat(apNames, " ");
              char temp[10];
              sprintf(temp, "%d:%s", i+1, truncSSID);
              strcat(apNames, temp);
              break;
            }
          }
        }
      }
      oled.drawStr(0, 62, apNames);
    } while (oled.nextPage());
  }
}

void drawChannelRecommendation() {
  // Calculate channel congestion scores
  int channelLoad[13] = {0};
  int channelScore[13];

  // Count APs per channel
  for (int i = 0; i < apCount; i++) {
    if (apList[i].primary >= 1 && apList[i].primary <= 13) {
      channelLoad[apList[i].primary - 1]++;
    }
  }

  // Calculate scores (lower is better)
  for (int i = 0; i < 13; i++) {
    channelScore[i] = channelLoad[i] * 10; // Weight by AP count
  }

  // Find top 3 best channels
  int bestCh[3] = {1, 6, 11}; // Default to standard non-overlapping
  int bestScores[3] = {999, 999, 999};

  for (int ch = 0; ch < 13; ch++) {
    for (int rank = 0; rank < 3; rank++) {
      if (channelScore[ch] < bestScores[rank]) {
        // Shift worse channels down
        for (int j = 2; j > rank; j--) {
          bestCh[j] = bestCh[j - 1];
          bestScores[j] = bestScores[j - 1];
        }
        bestCh[rank] = ch + 1;
        bestScores[rank] = channelScore[ch];
        break;
      }
    }
  }

  oled.firstPage();
  do {
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(5, 10, "BEST CHANNELS");

    oled.setFont(u8g2_font_5x7_tf);

    // Show top 3 recommendations
    for (int i = 0; i < 3; i++) {
      int y = 24 + (i * 12);
      int ch = bestCh[i];
      int aps = channelLoad[ch - 1];

      oled.setCursor(0, y);
      oled.printf("%d. CH %d (%d APs)", i + 1, ch, aps);
    }

    // Reasoning
    oled.setFont(u8g2_font_4x6_tf);
    oled.setCursor(0, 55);
    oled.printf("Total APs scanned: %d", apCount);
    oled.drawStr(85, 63, "BACK");
  } while (oled.nextPage());
}

void drawEnvironmentChange() {
  oled.firstPage();
  do {
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(5, 10, "ENV CHANGE");

    oled.setFont(u8g2_font_5x7_tf);

    if (!baseline.saved) {
      oled.setCursor(0, 25);
      oled.print("No baseline saved");
      oled.setCursor(0, 35);
      oled.print("Hold ACTION to save");
    } else {
      // Show changes from baseline
      int apDiff = currentSnapshot.totalAPs - baseline.totalAPs;
      int rssiDiff = currentSnapshot.avgRSSI - baseline.avgRSSI;

      oled.setCursor(0, 20);
      oled.printf("APs: %d (%+d)", currentSnapshot.totalAPs, apDiff);

      oled.setCursor(0, 29);
      oled.printf("RSSI: %ddB (%+d)", currentSnapshot.avgRSSI, rssiDiff);

      // Find biggest channel change
      int maxChChange = 0;
      int maxCh = 0;
      for (int i = 0; i < 13; i++) {
        int diff = abs((int)currentSnapshot.channelDist[i] - (int)baseline.channelDist[i]);
        if (diff > maxChChange) {
          maxChChange = diff;
          maxCh = i + 1;
        }
      }

      oled.setCursor(0, 38);
      if (maxChChange > 0) {
        oled.printf("CH%d: %+d APs", maxCh,
          (int)currentSnapshot.channelDist[maxCh-1] - (int)baseline.channelDist[maxCh-1]);
      } else {
        oled.print("Channels stable");
      }

      // Time since baseline
      uint32_t elapsed = (millis() - baseline.timestamp) / 1000;
      oled.setCursor(0, 47);
      oled.printf("Age: %lus ago", elapsed);
    }

    // Footer
    oled.drawLine(0, 54, 127, 54);
    oled.setFont(u8g2_font_4x6_tf);
    if (baseline.saved) {
      oled.drawStr(0, 61, "LONG=Save BACK=Menu");
    } else {
      oled.drawStr(15, 61, "BACK=Menu");
    }
  } while (oled.nextPage());
}

void drawQuickSnapshot() {
  // Quick snapshot of current RF environment
  oled.firstPage();
  do {
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(5, 10, "QUICK SNAPSHOT");

    oled.setFont(u8g2_font_5x7_tf);

    // WiFi Stats
    oled.setCursor(0, 22);
    oled.printf("WiFi APs: %d", apCount);

    // BLE Stats
    oled.setCursor(0, 31);
    uint8_t activeBLE = getActiveBLECount();
    oled.printf("BLE Devices: %d", activeBLE);

    // Average RSSI
    int avgRSSI = 0;
    if (apCount > 0) {
      long rssiSum = 0;
      for (int i = 0; i < apCount; i++) {
        rssiSum += apList[i].rssi;
      }
      avgRSSI = rssiSum / apCount;
    }
    oled.setCursor(0, 40);
    oled.printf("Avg RSSI: %d dBm", avgRSSI);

    // Channel with most APs
    int channelLoad[13] = {0};
    int maxLoad = 0;
    int busiestCh = 1;
    for (int i = 0; i < apCount; i++) {
      if (apList[i].primary >= 1 && apList[i].primary <= 13) {
        channelLoad[apList[i].primary - 1]++;
        if (channelLoad[apList[i].primary - 1] > maxLoad) {
          maxLoad = channelLoad[apList[i].primary - 1];
          busiestCh = apList[i].primary;
        }
      }
    }
    oled.setCursor(0, 49);
    oled.printf("Busiest: CH%d (%d)", busiestCh, maxLoad);

    // Footer
    oled.drawLine(0, 54, 127, 54);
    oled.setFont(u8g2_font_4x6_tf);
    oled.drawStr(30, 61, "BACK=Menu");
  } while (oled.nextPage());
}

void drawChannelScorecard() {
  // Visual quality/congestion score for all channels
  oled.firstPage();
  do {
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(5, 10, "CHANNEL SCORE");

    oled.setFont(u8g2_font_4x6_tf);

    // Count APs per channel
    int channelLoad[13] = {0};
    for (int i = 0; i < apCount; i++) {
      if (apList[i].primary >= 1 && apList[i].primary <= 13) {
        channelLoad[apList[i].primary - 1]++;
      }
    }

    // Draw channel bars (7 rows of 2 channels each, last row has 1)
    for (int row = 0; row < 7; row++) {
      uint8_t y = 16 + row * 6;

      // Left channel
      if (row * 2 < 13) {
        int ch = row * 2 + 1;
        int load = channelLoad[ch - 1];

        oled.setCursor(0, y + 5);
        oled.printf("CH%d", ch);

        // Bar (max 20 APs = full bar)
        uint8_t barW = min(load * 2, 20);
        oled.drawFrame(18, y, 22, 4);
        if (barW > 0) oled.drawBox(19, y + 1, barW, 2);

        // Count
        oled.setCursor(42, y + 5);
        oled.printf("%d", load);
      }

      // Right channel
      if (row * 2 + 1 < 13) {
        int ch = row * 2 + 2;
        int load = channelLoad[ch - 1];

        oled.setCursor(64, y + 5);
        oled.printf("CH%d", ch);

        // Bar
        uint8_t barW = min(load * 2, 20);
        oled.drawFrame(82, y, 22, 4);
        if (barW > 0) oled.drawBox(83, y + 1, barW, 2);

        // Count
        oled.setCursor(106, y + 5);
        oled.printf("%d", load);
      }
    }

    // Footer
    oled.drawLine(0, 57, 127, 57);
    oled.drawStr(30, 63, "BACK=Menu");
  } while (oled.nextPage());
}

/* ========== History Screens ========== */

void drawEventLog() {
  oled.firstPage();
  do {
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(25, 10, "EVENT LOG");

    if (eventCount == 0) {
      oled.setFont(u8g2_font_5x7_tf);
      oled.setCursor(15, 30);
      oled.print("No events logged");
    } else {
      oled.setFont(u8g2_font_4x6_tf);

      // Show up to 6 events
      for (int i = 0; i < min(6, (int)eventCount); i++) {
        int idx = eventScroll + i;
        if (idx >= eventCount) break;

        LogEvent* ev = &eventLog[idx];
        uint8_t y = 18 + (i * 7);

        // Highlight cursor
        if (i == eventCursor) {
          oled.drawBox(0, y - 6, 128, 7);
          oled.setDrawColor(0);
        }

        // Event type icon
        const char* icon = "?";
        if (ev->type == 0) icon = "D";  // Deauth
        else if (ev->type == 1) icon = "R";  // Rogue
        else if (ev->type == 2) icon = "T";  // Tracker

        oled.setCursor(0, y);
        oled.printf("%s:", icon);

        // Message (truncated)
        char msg[23];
        strncpy(msg, ev->message, 22);
        msg[22] = '\0';
        oled.setCursor(10, y);
        oled.print(msg);

        if (i == eventCursor) oled.setDrawColor(1);
      }

      // Scrollbar
      if (eventCount > 6) {
        if (eventScroll > 0) oled.drawStr(122, 18, "^");
        if (eventScroll + 6 < eventCount) oled.drawStr(122, 53, "v");
      }
    }

    // Footer
    oled.drawLine(0, 54, 127, 54);
    oled.setFont(u8g2_font_4x6_tf);
    oled.drawStr(30, 61, "BACK=Menu");
  } while (oled.nextPage());
}

void drawBaselineCompare() {
  oled.firstPage();
  do {
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(5, 10, "BASELINE VS NOW");

    oled.setFont(u8g2_font_4x6_tf);

    if (!baseline.saved) {
      oled.setCursor(0, 25);
      oled.print("No baseline saved");
      oled.setCursor(0, 35);
      oled.print("Use Env Change to save");
    } else {
      // Header
      oled.setCursor(0, 18);
      oled.print("Metric");
      oled.setCursor(60, 18);
      oled.print("Base");
      oled.setCursor(95, 18);
      oled.print("Now");

      // Total APs
      oled.setCursor(0, 26);
      oled.print("APs:");
      oled.setCursor(60, 26);
      oled.printf("%d", baseline.totalAPs);
      oled.setCursor(95, 26);
      oled.printf("%d", currentSnapshot.totalAPs);

      // Avg RSSI
      oled.setCursor(0, 34);
      oled.print("RSSI:");
      oled.setCursor(60, 34);
      oled.printf("%d", baseline.avgRSSI);
      oled.setCursor(95, 34);
      oled.printf("%d", currentSnapshot.avgRSSI);

      // Total Packets
      oled.setCursor(0, 42);
      oled.print("Pkts:");
      oled.setCursor(60, 42);
      oled.printf("%lu", baseline.totalPackets);
      oled.setCursor(95, 42);
      oled.printf("%lu", currentSnapshot.totalPackets);

      // Time
      uint32_t elapsed = (millis() - baseline.timestamp) / 1000;
      oled.setCursor(0, 50);
      oled.printf("Baseline: %lus ago", elapsed);
    }

    // Footer
    oled.drawLine(0, 54, 127, 54);
    oled.setFont(u8g2_font_4x6_tf);
    oled.drawStr(30, 61, "BACK=Menu");
  } while (oled.nextPage());
}

/* ========== System Screens ========== */

void drawBatteryPower() {
  oled.firstPage();
  do {
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(5, 10, "SYSTEM INFO");

    oled.setFont(u8g2_font_5x7_tf);

    // Uptime
    uint32_t uptime = millis() / 1000;
    uint32_t hours = uptime / 3600;
    uint32_t minutes = (uptime % 3600) / 60;
    uint32_t seconds = uptime % 60;

    oled.setCursor(0, 22);
    oled.print("Uptime:");
    oled.setCursor(0, 32);
    oled.printf("  %02luh %02lum %02lus", hours, minutes, seconds);

    // Free heap
    oled.setCursor(0, 42);
    oled.printf("Free RAM: %d KB", ESP.getFreeHeap() / 1024);

    // Flash size
    oled.setCursor(0, 52);
    oled.printf("Flash: %d MB", ESP.getFlashChipSize() / (1024 * 1024));

    // Footer
    oled.drawLine(0, 54, 127, 54);
    oled.setFont(u8g2_font_4x6_tf);
    oled.drawStr(30, 61, "BACK=Menu");
  } while (oled.nextPage());
}

void drawDisplaySettings() {
  oled.firstPage();
  do {
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(15, 10, "DISPLAY");

    oled.setFont(u8g2_font_5x7_tf);

    // RGB Brightness
    oled.setCursor(0, 22);
    if (displaySettingCursor == 0) oled.print(">");
    oled.setCursor(8, 22);
    oled.printf("RGB: %d%%", settings.rgbBrightness);

    // Sleep Timeout
    oled.setCursor(0, 32);
    if (displaySettingCursor == 1) oled.print(">");
    oled.setCursor(8, 32);
    if (settings.screenTimeout == 0) {
      oled.print("Sleep: Never");
    } else {
      oled.printf("Sleep: %ds", settings.screenTimeout);
    }

    // Instructions
    oled.setFont(u8g2_font_4x6_tf);
    oled.setCursor(0, 46);
    oled.print("SHORT=Next LONG=Change");

    // Footer
    oled.drawLine(0, 54, 127, 54);
    oled.drawStr(30, 61, "BACK=Save");
  } while (oled.nextPage());
}

void drawRadioControl() {
  oled.firstPage();
  do {
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(10, 10, "RADIO CONTROL");

    oled.setFont(u8g2_font_5x7_tf);

    // Current Channel
    oled.setCursor(0, 22);
    oled.printf("Monitor Channel: %d", currentChannel);

    // Channel bar
    oled.setFont(u8g2_font_4x6_tf);
    oled.drawFrame(0, 26, 128, 12);
    int chPos = ((currentChannel - 1) * 116) / 12 + 1;
    oled.drawBox(chPos, 27, 8, 10);

    oled.setCursor(0, 46);
    oled.print("1        6        11  13");

    // Instructions
    oled.setFont(u8g2_font_5x7_tf);
    oled.setCursor(0, 52);
    oled.print("SHORT=Next LONG=Prev");

    // Footer
    oled.drawLine(0, 54, 127, 54);
    oled.setFont(u8g2_font_4x6_tf);
    oled.drawStr(30, 61, "BACK=Menu");
  } while (oled.nextPage());
}

void drawAbout() {
  oled.firstPage();
  do {
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(35, 10, "ABOUT");

    oled.setFont(u8g2_font_5x7_tf);
    oled.drawStr(5, 25, "Pocket RF Tool");
    oled.drawStr(5, 35, "ESP32-C3/S3");
    oled.setCursor(5, 45);
    oled.printf("Version: %s", FW_VERSION);
    oled.drawStr(5, 55, "Wi-Fi + BLE");

    oled.setFont(u8g2_font_4x6_tf);
    oled.drawStr(25, 63, "BACK = Return");
  } while (oled.nextPage());
}

/* ========== Utility Screens ========== */

void drawHiddenSSID() {
  oled.firstPage();
  do {
    oled.setFont(u8g2_font_5x7_tf);
    oled.drawFrame(0, 0, 128, 9);
    oled.setCursor(2, 7);
    oled.printf("Hidden:%d", hiddenCount);

    if (hiddenCount == 0) {
      oled.setFont(u8g2_font_6x10_tf);
      oled.drawStr(10, 35, "Scanning...");
    } else {
      for (int row = 0; row < AP_VISIBLE; row++) {
        int idx = hiddenScroll + row;
        if (idx >= hiddenCount) break;
        oled.setCursor(0, 18 + row * 15);
        if (row == hiddenCursor) oled.print(">");
        oled.setFont(u8g2_font_5x7_tf);

        char ssidBuf[13] = {0};
        strncpy(ssidBuf, hiddenList[idx].ssid, 12);
        oled.printf("%s", ssidBuf);

        oled.setFont(u8g2_font_4x6_tf);
        oled.setCursor(80, 18 + row * 15);
        oled.printf("CH%d", hiddenList[idx].channel);

        oled.setCursor(110, 18 + row * 15);
        oled.printf("%d", hiddenList[idx].rssi);
      }
    }
  } while (oled.nextPage());
}

void drawStats() {
  uint32_t runtime = (millis() - sessionStart) / 1000;
  uint32_t hours = runtime / 3600;
  uint32_t mins = (runtime % 3600) / 60;
  uint32_t secs = runtime % 60;

  oled.firstPage();
  do {
    oled.setFont(u8g2_font_5x7_tf);
    oled.drawStr(0, 8, "SESSION STATISTICS");

    oled.setFont(u8g2_font_6x10_tf);
    oled.setCursor(0, 20);
    oled.printf("Time: %02d:%02d:%02d", hours, mins, secs);

    oled.setCursor(0, 30);
    oled.printf("Packets: %lu", totalPackets);

    oled.setCursor(0, 40);
    oled.printf("APs: %lu  Peak:%lu", totalAPsFound, peakPPS);

    oled.setCursor(0, 50);
    oled.printf("Deauths: %lu", totalDeauthDetected);

    if (deauthChannel > 0 && totalDeauthDetected > 10) {
      oled.setCursor(0, 60);
      oled.printf("Last on CH%d", deauthChannel);
    }

    oled.setFont(u8g2_font_4x6_tf);
    oled.drawStr(0, 63, "PRESS=Back HOLD=Reset");

  } while (oled.nextPage());
}

void drawRSSIMeter() {
  oled.firstPage();
  do {
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(25, 8, "RSSI METER");

    if (apCount > 0) {
      oled.setFont(u8g2_font_5x7_tf);

      // Get strongest AP
      wifi_ap_record_t* ap = &apList[0];

      // Display SSID
      char ssid[17];
      strncpy(ssid, (char*)ap->ssid, 16);
      ssid[16] = '\0';
      char buf[20];
      sprintf(buf, "%s", strlen((char*)ap->ssid) ? ssid : "<hidden>");
      oled.drawStr(0, 20, buf);

      // Display RSSI value
      sprintf(buf, "RSSI: %d dBm", ap->rssi);
      oled.drawStr(0, 32, buf);

      // Display channel
      sprintf(buf, "CH: %d", ap->primary);
      oled.drawStr(0, 42, buf);

      // Draw RSSI bar (0 to 100% scale, -90dBm to -30dBm)
      int barWidth = map(constrain(ap->rssi, -90, -30), -90, -30, 0, 120);
      oled.drawFrame(0, 50, 122, 8);
      oled.drawBox(1, 51, barWidth, 6);

    } else {
      oled.setFont(u8g2_font_5x7_tf);
      oled.drawStr(10, 35, "Scanning for APs...");
    }

    oled.setFont(u8g2_font_4x6_tf);
    oled.drawStr(90, 63, "BACK");

  } while (oled.nextPage());
}

void drawExport() {
  oled.firstPage();
  do {
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(20, 10, "EXPORT DATA");

    oled.setFont(u8g2_font_5x7_tf);

    // Export options
    oled.setCursor(0, 24);
    oled.print("SHORT = Export to Serial");

    oled.setCursor(0, 36);
    oled.printf("APs: %d  BLE: %d", apCount, getActiveBLECount());

    oled.setCursor(0, 46);
    oled.printf("Events: %d", eventCount);

    // Footer
    oled.drawLine(0, 54, 127, 54);
    oled.setFont(u8g2_font_4x6_tf);
    oled.drawStr(30, 61, "BACK=Menu");
  } while (oled.nextPage());
}

void drawPowerMode() {
  oled.firstPage();
  do {
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(20, 10, "POWER MODE");

    oled.setFont(u8g2_font_5x7_tf);

    // Power mode options
    const char* modes[] = {"Normal", "Eco", "Ultra"};
    const char* desc[] = {
      "Full performance",
      "Reduced scan rate",
      "Maximum battery"
    };

    for (int i = 0; i < 3; i++) {
      oled.setCursor(0, 24 + i * 12);
      if (settings.powerMode == i) oled.print(">");
      oled.setCursor(8, 24 + i * 12);
      oled.printf("%s", modes[i]);

      oled.setFont(u8g2_font_4x6_tf);
      oled.setCursor(8, 30 + i * 12);
      oled.printf("  %s", desc[i]);
      oled.setFont(u8g2_font_5x7_tf);
    }

    // Footer
    oled.drawLine(0, 54, 127, 54);
    oled.setFont(u8g2_font_4x6_tf);
    oled.drawStr(0, 61, "SHORT=Change BACK=Save");
  } while (oled.nextPage());
}

/* ========== Placeholder Helper ========== */

void drawPlaceholder(const char* title, const char* subtitle) {
  oled.firstPage();
  do {
    oled.setFont(u8g2_font_6x10_tf);
    uint8_t titleWidth = strlen(title) * 6;
    uint8_t titleX = (128 - titleWidth) / 2;
    oled.drawStr(titleX, 25, title);

    if (subtitle) {
      oled.setFont(u8g2_font_5x7_tf);
      uint8_t subWidth = strlen(subtitle) * 5;
      uint8_t subX = (128 - subWidth) / 2;
      oled.drawStr(subX, 40, subtitle);
    }

    oled.setFont(u8g2_font_4x6_tf);
    oled.drawStr(20, 58, "BACK = Return");
  } while (oled.nextPage());
}
