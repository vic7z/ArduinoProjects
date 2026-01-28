#ifndef POWER_H
#define POWER_H

#include "config.h"

/* ========== Power Management ========== */

// Update screen timeout and sleep state
void updateScreenTimeout(bool buttonPressed);

// Check if screen should go to sleep and handle sleep/wake
void handleScreenSleep();

/* ========== External Variables ========== */
extern uint32_t lastActivity;
extern bool screenSleeping;

#endif // POWER_H
