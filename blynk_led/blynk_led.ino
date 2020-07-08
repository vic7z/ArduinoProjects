#define led 12

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char auth[] = "";
char ssid[] = "potassium";
char pass[] = "okgoogle";

int brightness;
int status;

BLYNK_WRITE(V1){
  status=param.asInt();
}

BLYNK_WRITE(V2){
  brightness=param.asInt();
}

void setup() {
 pinMode(led,OUTPUT);
 Serial.begin(9600);
 Blynk.begin(auth,ssid,pass);
}

void loop() {

 Blynk.run();

 if(status){
   Serial.print("led on");
   analogWrite(led,brightness);
   Serial.print(brightness);
 }else
 {
   Serial.print("led off");
   analogWrite(led,0);
 }
 
}
