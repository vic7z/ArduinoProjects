#ifndef DISPLAY_H
#define DISPLAY_H

#include "config.h"
#include "screens.h"

/* ========== RGB LED Functions ========== */
void setRGB(uint32_t color);
void updateRGBStatus();

/* ========== Draw Helper Functions ========== */
void drawGrid(uint8_t x0, uint8_t y0, uint8_t width, uint8_t height);
void drawPatternBar(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t load);
void drawGenericMenu(const char* title, const char** items, uint8_t itemCount, uint8_t& cursor);
void drawPlaceholder(const char* title, const char* subtitle = nullptr);

/* ========== Menu Draw Functions ========== */
void drawMenu();
void drawSecurityMenu();
void drawUtilitiesMenu();
void drawLogsMenu();
void drawSystemMenu();

/* ========== Screen Draw Functions ========== */
void drawMonitor();
void drawAnalyzer();
void drawApList();
void drawApDetail();
void drawCompare();
void drawHiddenSSID();
void drawSecurity();
void drawStats();
void drawSettings();
void drawLogger();
void drawBLEScan();
void drawBLEDetail();

/* ========== Placeholder Draw Functions ========== */
void drawAutoMode();
void drawRFHealth();
void drawDeauthWatch();
void drawEvilTwinWatch();
void drawBLETrackerWatch();
void drawAlertSettings();
void drawQuickSnapshot();
void drawRSSIMeter();
void drawChannelScorecard();
void drawEventLog();
void drawExport();
void drawBattery();
void drawPowerMode();
void drawDisplaySettings();
void drawRadioControl();
void drawAbout();

#endif // DISPLAY_H
