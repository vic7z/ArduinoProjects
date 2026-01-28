#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <U8g2lib.h>
#include <Adafruit_NeoPixel.h>
#include <Preferences.h>

/* ========== Version ========== */
#define FW_VERSION "v1.2.1"
#define FW_DATE "2026-01-21"

/* ========== Hardware Configuration ========== */
#define BTN_ACTION 2
#define BTN_BACK_PIN 3
#define RGB_LED_PIN 10
#define RGB_LED_COUNT 1
#define RGB_ENABLED true

/* ========== RGB Colors ========== */
#define RGB_OFF     rgb.Color(0, 0, 0)
#define RGB_GREEN   rgb.Color(0, 50, 0)
#define RGB_BLUE    rgb.Color(0, 0, 50)
#define RGB_YELLOW  rgb.Color(50, 50, 0)
#define RGB_ORANGE  rgb.Color(50, 15, 0)
#define RGB_RED     rgb.Color(50, 0, 0)
#define RGB_PURPLE  rgb.Color(25, 0, 25)
#define RGB_WHITE   rgb.Color(20, 20, 20)
#define RGB_CYAN    rgb.Color(0, 25, 25)

/* ========== Button Configuration ========== */
#define DEBOUNCE_MS 30
#define SHORT_PRESS_MS 300
#define LONG_PRESS_MS 700
#define DOUBLE_CLICK_MS 600  // Max time between clicks for double-click (increased for easier use)

enum ButtonEvent { BTN_NONE, BTN_SHORT, BTN_LONG, BTN_BACK, BTN_BACK_LONG };

/* ========== Wi-Fi Configuration ========== */
#define MAX_CHANNEL 13
#define HISTORY_SIZE 128
#define MAX_APS 20
#define AP_VISIBLE 3
#define MAX_HIDDEN_SSIDS 10
#define MAX_SSID_LEN 32
#define MAX_ROGUE_APS 5

/* ========== BLE Configuration ========== */
#define MAX_BLE_DEVICES 20
#define BLE_VISIBLE 3
#define MAX_EVENTS 10
#define WALK_HISTORY_SIZE 60

/* ========== RSSI History Configuration ========== */
#define RSSI_HISTORY_SIZE 50
#define MAX_TRACKED_APS 3

/* ========== Device Monitor Configuration ========== */
#define MAX_MONITORED_DEVICES 15
#define DEVICE_TIMEOUT_MS 30000  // Consider device gone after 30 seconds

/* ========== Data Structures ========== */

// Settings Structure
struct Settings {
  uint8_t scanSpeed;      // 0=fast, 1=normal, 2=slow
  int8_t rssiThreshold;   // Alert threshold
  uint8_t rgbBrightness;  // 0-100
  bool autoRefresh;
  bool csvLogging;
  uint8_t deauthThreshold;  // Deauth packets/sec to trigger alert (default: 10)
  uint16_t screenTimeout;   // Screen timeout in seconds (0=never, default: 60)
  uint8_t powerMode;        // 0=normal, 1=eco, 2=ultra (default: 0)
};

// Hidden SSID Structure
struct HiddenSSID {
  char ssid[MAX_SSID_LEN + 1];
  int8_t rssi;
  uint8_t channel;
  bool active;
};

// Rogue AP Structure
struct RogueAP {
  char ssid[MAX_SSID_LEN + 1];
  uint8_t bssid1[6];
  uint8_t bssid2[6];
  bool active;
};

// BLE Device Structure
struct BLEDeviceInfo {
  String address;
  String name;
  int rssi;
  bool isActive;
  uint32_t lastSeen;
  uint8_t advType; // 0=unknown, 1=iBeacon, 2=Eddystone
  bool hasName;
};

// MAC Vendor Structure
struct Vendor {
  uint8_t oui[3];
  const char* name;
};

// Event Log Structure
struct LogEvent {
  uint32_t timestamp;  // millis() when event occurred
  uint8_t type;        // 0=deauth, 1=rogue, 2=tracker
  char message[40];    // Event details
  bool active;
};

// Baseline Snapshot Structure
struct Baseline {
  uint32_t timestamp;
  uint16_t totalAPs;
  int16_t avgRSSI;
  uint8_t channelDist[13];  // APs per channel
  uint32_t totalPackets;
  bool saved;
};

// Device Monitor Structure
struct MonitoredDevice {
  uint8_t type;  // 0=WiFi, 1=BLE
  uint8_t bssid[6];  // For WiFi AP
  char bleAddr[18];  // For BLE device
  char name[33];     // SSID or BLE name
  int8_t rssi;
  uint8_t channel;   // Only for WiFi
  uint32_t firstSeen;
  uint32_t lastSeen;
  uint16_t seenCount;
  bool isPresent;    // Currently visible
  bool active;       // Slot is in use
};

// RSSI History Structure
struct RSSIHistory {
  uint8_t bssid[6];
  int8_t rssiSamples[RSSI_HISTORY_SIZE];
  uint8_t sampleIndex;
  bool active;
};

/* ========== Global Hardware Objects ========== */
extern U8G2_SSD1306_128X64_NONAME_1_HW_I2C oled;
extern Adafruit_NeoPixel rgb;
extern Preferences prefs;

/* ========== Global Settings ========== */
extern Settings settings;

/* ========== Event Log & Baseline ========== */
extern LogEvent eventLog[MAX_EVENTS];
extern uint8_t eventCount;
extern uint8_t eventCursor;
extern uint8_t eventScroll;
extern Baseline baseline;
extern Baseline currentSnapshot;

/* ========== Device Monitor ========== */
extern MonitoredDevice monitoredDevices[MAX_MONITORED_DEVICES];
extern uint8_t monitoredDeviceCount;
extern uint8_t deviceCursor;
extern uint8_t deviceScroll;
extern uint8_t deviceSelectedIndex;

#endif // CONFIG_H
