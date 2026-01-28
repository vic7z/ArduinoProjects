#ifndef SCREENS_H
#define SCREENS_H

/* ========== Screen State Machine ========== */
/* Redesigned for purpose-centric navigation */
enum Screen {
  // Main Menu
  SCREEN_MENU,

  // Main Menu Direct Items
  SCREEN_AUTO_WATCH,
  SCREEN_RF_HEALTH,
  SCREEN_MONITOR,         // Live Monitor
  SCREEN_ANALYZER,        // Channel Analyzer
  SCREEN_DEVICE_MONITOR,

  // AP Scanner (direct access)
  SCREEN_AP_LIST,
  SCREEN_AP_DETAIL,
  SCREEN_AP_WALK_TEST,

  // BLE Monitor (direct access)
  SCREEN_BLE_SCAN,
  SCREEN_BLE_DETAIL,
  SCREEN_BLE_WALK_TEST,

  // Security (submenu)
  SCREEN_SECURITY_MENU,
  SCREEN_DEAUTH_WATCH,
  SCREEN_ROGUE_AP_WATCH,
  SCREEN_BLE_TRACKER_WATCH,
  SCREEN_ALERT_SETTINGS,

  // Insights (submenu)
  SCREEN_INSIGHTS_MENU,
  SCREEN_WHY_IS_IT_SLOW,
  SCREEN_CHANNEL_RECOMMENDATION,
  SCREEN_ENVIRONMENT_CHANGE,
  SCREEN_QUICK_SNAPSHOT,
  SCREEN_CHANNEL_SCORECARD,

  // History (submenu)
  SCREEN_HISTORY_MENU,
  SCREEN_EVENT_LOG,
  SCREEN_BASELINE_COMPARE,
  SCREEN_EXPORT,

  // System (submenu)
  SCREEN_SYSTEM_MENU,
  SCREEN_BATTERY_POWER,
  SCREEN_DISPLAY_SETTINGS,
  SCREEN_RADIO_CONTROL,
  SCREEN_POWER_MODE,
  SCREEN_ABOUT,

  // Device Monitor Detail
  SCREEN_DEVICE_DETAIL,

  // Legacy/Internal (keep for compatibility)
  SCREEN_COMPARE,
  SCREEN_STATS,
  SCREEN_HIDDEN_SSID
};

extern Screen currentScreen;

#endif // SCREENS_H
