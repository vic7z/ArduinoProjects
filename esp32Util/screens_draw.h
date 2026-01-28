#ifndef SCREENS_DRAW_H
#define SCREENS_DRAW_H

#include "config.h"
#include "screens.h"

/* ========== Menu Drawing ========== */
void drawMenu();
void drawSecurityMenu();
void drawInsightsMenu();
void drawHistoryMenu();
void drawSystemMenu();

/* ========== Main Screens ========== */
void drawAutoWatch();
void drawRFHealth();
void drawMonitor();
void drawAnalyzer();
void drawDeviceMonitor();
void drawDeviceDetail();

/* ========== AP Scanner Screens ========== */
void drawApList();
void drawApDetail();
void drawAPWalkTest();
void drawCompare();

/* ========== BLE Scanner Screens ========== */
void drawBLEScan();
void drawBLEDetail();
void drawBLEWalkTest();

/* ========== Security Screens ========== */
void drawDeauthWatch();
void drawRogueAPWatch();
void drawBLETrackerWatch();
void drawAlertSettings();

/* ========== Insights Screens ========== */
void drawWhyIsItSlow();
void drawChannelRecommendation();
void drawEnvironmentChange();

/* ========== History Screens ========== */
void drawEventLog();
void drawBaselineCompare();

/* ========== System Screens ========== */
void drawBatteryPower();
void drawDisplaySettings();
void drawRadioControl();
void drawAbout();

/* ========== Utility Screens ========== */
void drawHiddenSSID();
void drawStats();
void drawQuickSnapshot();
void drawRSSIMeter();
void drawChannelScorecard();
void drawExport();
void drawPowerMode();

/* ========== Placeholder Helper ========== */
void drawPlaceholder(const char* title, const char* subtitle);

/* ========== Utility Functions ========== */
void takeSnapshot(Baseline* snap);

#endif // SCREENS_DRAW_H
