#ifndef MENU_H
#define MENU_H

#include "config.h"
#include "screens.h"

/* ========== Menu Data ========== */
/* Redesigned menu structure - purpose-centric navigation */

// Main Menu
extern const char* mainMenuItems[];
extern uint8_t mainMenuIndex;
extern const uint8_t MAIN_MENU_SIZE;

// Security Menu
extern const char* securityMenuItems[];
extern uint8_t securityMenuIndex;
extern const uint8_t SECURITY_MENU_SIZE;

// Insights Menu
extern const char* insightsMenuItems[];
extern uint8_t insightsMenuIndex;
extern const uint8_t INSIGHTS_MENU_SIZE;

// History Menu
extern const char* historyMenuItems[];
extern uint8_t historyMenuIndex;
extern const uint8_t HISTORY_MENU_SIZE;

// System Menu
extern const char* systemMenuItems[];
extern uint8_t systemMenuIndex;
extern const uint8_t SYSTEM_MENU_SIZE;

#endif // MENU_H
