#include "menu.h"

/* ========== Menu Data ========== */
/* Redesigned for purpose-centric navigation */

// Main Menu
const char* mainMenuItems[] = {
  "Auto Watch",
  "RF Health",
  "Live Monitor",
  "Channel Analyzer",
  "Device Monitor",
  "AP Scanner",
  "BLE Monitor",
  "Security",
  "Insights",
  "History",
  "System"
};
uint8_t mainMenuIndex = 0;
const uint8_t MAIN_MENU_SIZE = 11;

// Security Menu
const char* securityMenuItems[] = {
  "Deauth Watch",
  "Rogue AP Watch",
  "BLE Tracker Watch",
  "Alert Settings"
};
uint8_t securityMenuIndex = 0;
const uint8_t SECURITY_MENU_SIZE = 4;

// Insights Menu
const char* insightsMenuItems[] = {
  "Why Is It Slow?",
  "Channel Recommend",
  "Environment Change",
  "Quick Snapshot",
  "Channel Scorecard"
};
uint8_t insightsMenuIndex = 0;
const uint8_t INSIGHTS_MENU_SIZE = 5;

// History Menu
const char* historyMenuItems[] = {
  "Event Log",
  "Baseline Compare",
  "Export Data"
};
uint8_t historyMenuIndex = 0;
const uint8_t HISTORY_MENU_SIZE = 3;

// System Menu
const char* systemMenuItems[] = {
  "Battery & Power",
  "Display",
  "Radio Control",
  "Power Mode",
  "About"
};
uint8_t systemMenuIndex = 0;
const uint8_t SYSTEM_MENU_SIZE = 5;
