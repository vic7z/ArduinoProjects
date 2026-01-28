#include "display.h"
#include "wifi_scanner.h"
#include "ble_scanner.h"
#include "menu.h"
#include "utils.h"
#include "security.h"
#include "power.h"

/* ========== RGB LED Functions ========== */
void setRGB(uint32_t color) {
  if (!RGB_ENABLED) return;

  // Apply brightness scaling
  uint8_t r = (color >> 16) & 0xFF;
  uint8_t g = (color >> 8) & 0xFF;
  uint8_t b = color & 0xFF;

  r = (r * settings.rgbBrightness) / 100;
  g = (g * settings.rgbBrightness) / 100;
  b = (b * settings.rgbBrightness) / 100;

  rgb.setPixelColor(0, rgb.Color(r, g, b));
  rgb.show();
}

void updateRGBStatus() {
  if (!RGB_ENABLED) return;

  // Don't update LED while screen is sleeping
  if (screenSleeping) return;

  if (attackActive) {
    setRGB(RGB_RED);
  } else if (signalAlert) {
    setRGB(RGB_CYAN);
  } else if (currentScreen == SCREEN_MONITOR) {
    uint8_t load = liveLoad();
    if (load > 70) setRGB(RGB_ORANGE);
    else if (load > 40) setRGB(RGB_YELLOW);
    else setRGB(RGB_GREEN);
  } else if (currentScreen == SCREEN_ANALYZER) {
    setRGB(RGB_BLUE);
  } else if (currentScreen == SCREEN_HIDDEN_SSID) {
    setRGB(RGB_PURPLE);
  } else if (currentScreen == SCREEN_BLE_SCAN) {
    setRGB(RGB_CYAN);
  } else {
    setRGB(RGB_WHITE);
  }
}

/* ========== Graph Helper Functions ========== */
void drawGrid(uint8_t x0, uint8_t y0, uint8_t width, uint8_t height) {
  for (int i = 1; i < 4; i++) {
    int y = y0 + (height * i) / 4;
    for (int x = x0; x < x0 + width; x += 4) {
      oled.drawPixel(x, y);
    }
  }
  for (int x = x0 + 32; x < x0 + width; x += 32) {
    for (int y = y0; y < y0 + height; y += 4) {
      oled.drawPixel(x, y);
    }
  }
}

void drawPatternBar(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t load) {
  if (height == 0) return;
  oled.drawFrame(x, y, width, height);

  if (load < 20) {
    for (int py = y + 1; py < y + height - 1; py++) {
      for (int px = x + 1; px < x + width - 1; px++) {
        if ((px + py) % 4 == 0) oled.drawPixel(px, py);
      }
    }
  } else if (load < 40) {
    for (int i = 0; i < height + width; i += 3) {
      for (int j = 0; j < height; j++) {
        int px = x + i - j;
        int py = y + j;
        if (px >= x + 1 && px < x + width - 1 && py >= y + 1 && py < y + height - 1) {
          oled.drawPixel(px, py);
        }
      }
    }
  } else if (load < 70) {
    for (int py = y + 1; py < y + height - 1; py++) {
      for (int px = x + 1; px < x + width - 1; px++) {
        if ((px + py) % 2 == 0) oled.drawPixel(px, py);
      }
    }
  } else {
    oled.drawBox(x + 1, y + 1, width - 2, height - 2);
  }
}

// Note: All draw* functions are kept in esp32Util.ino
// Arduino IDE automatically links .ino files together
