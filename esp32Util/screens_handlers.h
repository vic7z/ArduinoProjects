#ifndef SCREENS_HANDLERS_H
#define SCREENS_HANDLERS_H

#include "config.h"
#include "screens.h"

/* ========== Screen Handler Functions ========== */
/* These functions handle button events and screen updates for each screen */

// Menu handlers
void handleMainMenu(ButtonEvent ev);
void handleSecurityMenu(ButtonEvent ev);
void handleInsightsMenu(ButtonEvent ev);
void handleHistoryMenu(ButtonEvent ev);
void handleSystemMenu(ButtonEvent ev);

// Main screens handlers
void handleAutoWatch(ButtonEvent ev);
void handleRFHealth(ButtonEvent ev);
void handleMonitor(ButtonEvent ev);
void handleAnalyzer(ButtonEvent ev);
void handleDeviceMonitor(ButtonEvent ev);
void handleDeviceDetail(ButtonEvent ev);

// AP Scanner handlers
void handleApList(ButtonEvent ev);
void handleApDetail(ButtonEvent ev);
void handleAPWalkTest(ButtonEvent ev);
void handleCompare(ButtonEvent ev);
void handleHiddenSSID(ButtonEvent ev);

// BLE Scanner handlers
void handleBLEScan(ButtonEvent ev);
void handleBLEDetail(ButtonEvent ev);
void handleBLEWalkTest(ButtonEvent ev);

// Security handlers
void handleDeauthWatch(ButtonEvent ev);
void handleRogueAPWatch(ButtonEvent ev);
void handleBLETrackerWatch(ButtonEvent ev);
void handleAlertSettings(ButtonEvent ev);

// Insights handlers
void handleWhyIsItSlow(ButtonEvent ev);
void handleChannelRecommendation(ButtonEvent ev);
void handleEnvironmentChange(ButtonEvent ev);
void handleQuickSnapshot(ButtonEvent ev);
void handleChannelScorecard(ButtonEvent ev);

// History handlers
void handleEventLog(ButtonEvent ev);
void handleBaselineCompare(ButtonEvent ev);
void handleExport(ButtonEvent ev);

// System handlers
void handleBatteryPower(ButtonEvent ev);
void handleDisplaySettings(ButtonEvent ev);
void handleRadioControl(ButtonEvent ev);
void handlePowerMode(ButtonEvent ev);
void handleAbout(ButtonEvent ev);

// Utility handlers
void handleStats(ButtonEvent ev);

#endif // SCREENS_HANDLERS_H
