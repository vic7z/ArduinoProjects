
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include "Ubidots.h"

const char* UBIDOTS_TOKEN = "";  
const char* WIFI_SSID = "";      
const char* WIFI_PASS = "";    

float Temperature,preassure,altitude,battery;
bool bufferSent = false;

Adafruit_BMP280 bmp; 
Ubidots ubidots(UBIDOTS_TOKEN, UBI_HTTP);
ADC_MODE(ADC_VCC);

void setup() {
  Serial.begin(9600);
  ubidots.wifiConnect(WIFI_SSID, WIFI_PASS);

  if (!bmp.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
   //sleeps forever
    ESP.deepSleep(0);
  }

  
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,    
                  Adafruit_BMP280::SAMPLING_X2,    
                  Adafruit_BMP280::SAMPLING_X16,    
                  Adafruit_BMP280::FILTER_X16,      
                  Adafruit_BMP280::STANDBY_MS_500); 
}

void loop() {
    Temperature =bmp.readTemperature();
    preassure =bmp.readPressure();
    altitude =bmp.readAltitude(1013.25);

    Serial.print(F("Temperature = "));
   
    Serial.print(Temperature);
    Serial.println(" *C");
    ubidots.add("Temperature",Temperature); 

    Serial.print(F("Pressure = "));
    Serial.print(preassure);
    Serial.println(" Pa");
    ubidots.add("preassure",preassure);

    Serial.print(F("Approx altitude = "));
    Serial.print(altitude); 
    Serial.println(" m");
   

    ubidots.add("altitude",altitude);
    battery=ESP.getVcc()/1000;
    ubidots.add("battery",battery);
    Serial.println(battery);


    bufferSent = ubidots.send();



  if (bufferSent) {
    Serial.println("Values sent by the device");
    ESP.deepSleep(300e6); //5*60
  }

    delay(2000);
}
