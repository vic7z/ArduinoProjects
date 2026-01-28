#include "settings.h"

/* ========== Settings Variables ========== */
uint8_t settingsCursor = 0;
const uint8_t SETTINGS_COUNT = 5;

/* ========== Settings Management ========== */
void loadSettings() {
  prefs.begin("wifianalyzer", false);
  settings.scanSpeed = prefs.getUChar("scanSpeed", 1);
  settings.rssiThreshold = prefs.getChar("rssiThresh", -70);
  settings.rgbBrightness = prefs.getUChar("rgbBright", 50);
  settings.autoRefresh = prefs.getBool("autoRefresh", true);
  settings.csvLogging = prefs.getBool("csvLog", false);
  settings.deauthThreshold = prefs.getUChar("deauthThresh", 10);
  settings.screenTimeout = prefs.getUShort("screenTimeout", 60);
  settings.powerMode = prefs.getUChar("powerMode", 0);
}

void saveSettings() {
  prefs.putUChar("scanSpeed", settings.scanSpeed);
  prefs.putChar("rssiThresh", settings.rssiThreshold);
  prefs.putUChar("rgbBright", settings.rgbBrightness);
  prefs.putBool("autoRefresh", settings.autoRefresh);
  prefs.putBool("csvLog", settings.csvLogging);
  prefs.putUChar("deauthThresh", settings.deauthThreshold);
  prefs.putUShort("screenTimeout", settings.screenTimeout);
  prefs.putUChar("powerMode", settings.powerMode);
}
