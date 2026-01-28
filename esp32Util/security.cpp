#include "security.h"
#include "wifi_scanner.h"

/* ========== Rogue AP Data ========== */
RogueAP rogueList[MAX_ROGUE_APS];
uint8_t rogueCount = 0;

/* ========== Rogue AP Detection ========== */
void detectRogueAPs() {
  rogueCount = 0;
  for (int i = 0; i < apCount && rogueCount < MAX_ROGUE_APS; i++) {
    for (int j = i + 1; j < apCount; j++) {
      if (strcmp((char*)apList[i].ssid, (char*)apList[j].ssid) == 0 &&
          memcmp(apList[i].bssid, apList[j].bssid, 6) != 0) {
        strcpy(rogueList[rogueCount].ssid, (char*)apList[i].ssid);
        memcpy(rogueList[rogueCount].bssid1, apList[i].bssid, 6);
        memcpy(rogueList[rogueCount].bssid2, apList[j].bssid, 6);
        rogueList[rogueCount].active = true;
        rogueCount++;
        break;
      }
    }
  }
}
