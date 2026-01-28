#include "alerts.h"
#include "display.h"

/* ========== Alert State Variables ========== */
uint8_t alertLevel = 0;          // 0=normal, 1=warning, 2=critical
uint32_t lastAlertBlink = 0;
bool alertBlinkState = false;
bool signalAlert = false;
uint8_t alertSettingIndex = 0;

/* ========== Event Logging ========== */

void logEvent(uint8_t type, const char* msg) {
  // Shift old events if full
  if (eventCount >= MAX_EVENTS) {
    for (int i = 0; i < MAX_EVENTS - 1; i++) {
      eventLog[i] = eventLog[i + 1];
    }
    eventCount = MAX_EVENTS - 1;
  }

  // Add new event
  eventLog[eventCount].timestamp = millis();
  eventLog[eventCount].type = type;
  strncpy(eventLog[eventCount].message, msg, 39);
  eventLog[eventCount].message[39] = '\0';
  eventLog[eventCount].active = true;
  eventCount++;
}

/* ========== Alert LED Management ========== */

void updateAlertLED() {
  if (!RGB_ENABLED) return;

  // Blink interval based on alert level
  uint32_t blinkInterval = (alertLevel == 2) ? 200 : 500;  // Fast blink for critical, slow for warning

  if (millis() - lastAlertBlink > blinkInterval) {
    lastAlertBlink = millis();
    alertBlinkState = !alertBlinkState;

    if (alertLevel == 0) {
      // Normal - dim green
      setRGB(rgb.Color(0, 10, 0));
    } else if (alertLevel == 1) {
      // Warning - blink orange
      if (alertBlinkState) {
        setRGB(RGB_ORANGE);
      } else {
        setRGB(rgb.Color(10, 3, 0));  // Dim orange
      }
    } else if (alertLevel == 2) {
      // Critical - blink red
      if (alertBlinkState) {
        setRGB(RGB_RED);
      } else {
        setRGB(rgb.Color(10, 0, 0));  // Dim red
      }
    }
  }
}
