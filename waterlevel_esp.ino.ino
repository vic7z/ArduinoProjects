
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#define soil A0
BlynkTimer timer;

char auth[] = "ee369f228adf42f584f9dfdb9103854e";


char ssid[] = "spaceship";
char pass[] = "88888888";

void moister(){
  
  int mos;
  mos = analogRead(soil);
    Serial.print(mos);
  mos = map(mos,1024,505,0,100);
  Blynk.virtualWrite(V5, mos);

  
}



void setup()
{
  
  Serial.begin(9600);
  pinMode(soil,INPUT);
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(1000L, moister);
  
  
}

void loop()
{
  Blynk.run();
  timer.run();
}
