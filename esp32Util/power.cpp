#include "power.h"
#include "display.h"

/* ========== Power State Variables ========== */
uint32_t lastActivity = 0;
bool screenSleeping = false;

/* ========== Screen Timeout Management ========== */

void updateScreenTimeout(bool buttonPressed) {
  if (buttonPressed) {
    lastActivity = millis();
    if (screenSleeping) {
      // Wake up screen
      screenSleeping = false;
      oled.setPowerSave(0);  // Turn on OLED
      setRGB(RGB_GREEN);     // Turn on LED
    }
  }
}

void handleScreenSleep() {
  // Check if should sleep
  if (settings.screenTimeout > 0 && !screenSleeping) {
    if (millis() - lastActivity > (settings.screenTimeout * 1000UL)) {
      screenSleeping = true;
      oled.setPowerSave(1);  // Turn off OLED
      setRGB(RGB_OFF);       // Turn off LED
    }
  }
}
