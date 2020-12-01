
#include "HX711.h"
#include "Ubidots.h"
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

#define calibration_factor -7050.0 //This value is obtained using the SparkFun_HX711_Calibration sketch
#define I2C_ADDRESS 0x3C          //oled address

#define LOADCELL_DOUT_PIN  13   //pin D7
#define LOADCELL_SCK_PIN  15    //pin D8
#define booklet 0.05          //weight of one booklet


const char* UBIDOTS_TOKEN = "BBFF-wzk5MUWVns8uT1WTsXJoS27kJzfS0g";  // Put here your Ubidots TOKEN
const char* WIFI_SSID = "Mi";      // Put here your Wi-Fi SSID
const char* WIFI_PASS = "";      // Put here your Wi-Fi password


Ubidots ubidots(UBIDOTS_TOKEN, UBI_HTTP);
HX711 scale;
SSD1306AsciiWire oled;

void setup() {
  Serial.begin(9600);
  ubidots.wifiConnect(WIFI_SSID, WIFI_PASS);
  Wire.begin();
  Wire.setClock(400000L);
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(System5x7);

  
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calibration_factor);
  scale.tare(); 

  Serial.println("Readings:");
  oled.clear();
  oled.set2X();
  oled.println("reading");
  
  oled.println("");
  oled.clear();
}

void loop() {
  Serial.print("Reading: ");
  float weight=scale.get_units();
  Serial.print(weight, 1); 
  Serial.print(" kg");
 

  if(weight>0){
     ubidots.add("weight",weight);
     int totalbooklet=weight/booklet;
     oled.print("weight:");
     oled.print(weight);
     oled.println(" kg");
     oled.print("booklet:");
     oled.print(totalbooklet);
     oled.println();
     
     
  }
 
 
 
;
  Serial.println();
    bool bufferSent = false;
  bufferSent = ubidots.send(); 

  if (bufferSent) {
  
    Serial.println("Values sent by the device");
  }

  delay(100);
   oled.clear();
  
  
}
