#ifndef SECURITY_H
#define SECURITY_H

#include "config.h"

/* ========== Rogue AP Detection ========== */
extern RogueAP rogueList[MAX_ROGUE_APS];
extern uint8_t rogueCount;

void detectRogueAPs();

#endif // SECURITY_H
