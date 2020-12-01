

#include "Ubidots.h"
#include "HX711.h"
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"


#define calibration_factor 600.0
#define I2C_ADDRESS 0x3C

#define LOADCELL_DOUT_PIN  13
#define LOADCELL_SCK_PIN  15

HX711 scale;
SSD1306AsciiWire oled;

const char* UBIDOTS_TOKEN = "BBFF-wzk5MUWVns8uT1WTsXJoS27kJzfS0g";  
const char* WIFI_SSID = "Mi";      
const char* WIFI_PASS = "";      
Ubidots ubidots(UBIDOTS_TOKEN, UBI_HTTP);


void setup() {
  Serial.begin(115200);
  ubidots.wifiConnect(WIFI_SSID, WIFI_PASS);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calibration_factor);
  scale.tare();
  Wire.begin();
  Wire.setClock(400000L);
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(System5x7);
  oled.clear();
  oled.set1X();
  oled.println("initializing...");

 
}

void loop() {

 float weight =scale.get_units();
  
 ubidots.add("Weight", weight); 
 oled.print("weight=" );
 oled.println(weight);

   Serial.println(scale.get_units());

  bool bufferSent = false;
  bufferSent = ubidots.send();  

  if (bufferSent) {
   
    Serial.println("Values sent by the device");
  }

  delay(500);
}