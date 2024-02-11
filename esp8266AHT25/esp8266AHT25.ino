#include <Wire.h>
#include <AHTxx.h>
#include "Ubidots.h"

const char* UBIDOTS_TOKEN = "";  
const char* WIFI_SSID = "";      
const char* WIFI_PASS = "";    

float Temperature,humidity,battery;
bool bufferSent = false;

Ubidots ubidots(UBIDOTS_TOKEN, UBI_HTTP);
AHTxx aht20(AHTXX_ADDRESS_X38, AHT2x_SENSOR); 
ADC_MODE(ADC_VCC);

void setup() {
  Serial.begin(9600);

  int count = 0;
  while (WiFi.status() != WL_CONNECTED && count < 30) {
    delay(1000);
    Serial.print(".");
    count++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi");
      ubidots.wifiConnect(WIFI_SSID, WIFI_PASS);
  } else {
    ESP.deepSleep(300e6);
  }

  while (aht20.begin() != true)
  {
    Serial.println(F("AHT2x not connected or fail to load calibration coefficient")); 
    delay(5000);
  }
  Serial.println(F("AHT20 OK"));

}

void loop() {
    Temperature =aht20.readTemperature();
    humidity=aht20.readHumidity();

    Serial.print(F("Temperature = "));
   
    Serial.print(Temperature);
    Serial.println(" *C");
    ubidots.add("Temperature",Temperature); 

    Serial.print(F("humidity = "));
    Serial.print(humidity);
    ubidots.add("Humidity",humidity);

   
   

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