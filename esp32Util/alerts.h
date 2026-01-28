#ifndef ALERTS_H
#define ALERTS_H

#include "config.h"

/* ========== Alert Management ========== */

// Log an event to the event log
void logEvent(uint8_t type, const char* msg);

// Update the alert LED based on current alert level
void updateAlertLED();

/* ========== External Variables ========== */
extern uint8_t alertLevel;  // 0=normal, 1=warning, 2=critical
extern uint32_t lastAlertBlink;
extern bool alertBlinkState;
extern bool signalAlert;
extern uint8_t alertSettingIndex;

#endif // ALERTS_H
