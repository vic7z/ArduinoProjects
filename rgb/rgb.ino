

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

int r,g,b;

char auth[] = "e0fc9ccc949d4848bfa4ff4e0967ac24";


char ssid[] = "spaceship";
char pass[] = "88888888";


BLYNK_WRITE(V1) // Widget WRITEs to Virtual Pin V1
{   
  r = param[0].asInt(); 
  g = param[1].asInt(); 
  b = param[2].asInt(); 
 
  
  
}






void setup()
{
 
  Serial.begin(9600);

  Blynk.begin(auth, ssid, pass);
  pinMode(16,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(4,OUTPUT);
}





void loop()
{
  Blynk.run();
  analogWrite(16,r);
  analogWrite(5,g);
  analogWrite(4,b);
  Serial.println(r);
}
