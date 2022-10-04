
#define BLYNK_PRINT Serial
#define soilsensor A0
#define pumb 4

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char auth[] = "";


char ssid[] = "";
char pass[] = "";
float moisture;
int sensor;

int pumbremote=0;
int pumbspeed=0;


BlynkTimer timer;

void sendsensor(){
    sensor= analogRead(soilsensor);
    moisture = ((sensor/1023.0)*100);
    
    Blynk.virtualWrite(V2,sensor);
    Blynk.virtualWrite(V3,moisture);



}

void setup()
{
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(1000L,sendsensor);
}

BLYNK_WRITE(V6){
  pumbremote = param.asInt();
}

BLYNK_WRITE(V5){
  pumbspeed = param.asInt();
}

void loop()
{
  Blynk.run();
  timer.run();
  Serial.println(pumbspeed);
  Serial.println(pumbremote);
  if(pumbremote==0){

    if(moisture<50.0){
        analogWrite(pumb,125);
        Blynk.notify("soil moisture level is too low");
        Serial.println("moisture level low");

    }else
    {
        analogWrite(pumb,0);
    }
    
  }else if(pumbremote==1){
    analogWrite(pumb,pumbspeed);
    Serial.print("remote pumb override");

  }
}
