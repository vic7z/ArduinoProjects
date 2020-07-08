#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <FastLED.h>
#define lednum 28;
#define ledpin 7;

CRGB leds[NUM_LEDS];

int r,g,b;
int mode,speed;

char auth[] = "e0fc9ccc949d4848bfa4ff4e0967ac24";


char ssid[] = "spaceship";
char pass[] = "88888888";


BLYNK_WRITE(V1) 
{   
  r = param[0].asInt(); 
  g = param[1].asInt(); 
  b = param[2].asInt();  
}
BLYNK_WRITE(V2){
  mode = param.asInt();
}
BLYNK_WRITE(V3){
  speed=param.asInt();
}






void setup()
{
 
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
}





void loop()
{
  Blynk.run();
  if(mode=1){
    mode1(r,g,b)
  }
  
}

void mode1(int r,int g,int b){
for (int i = 0; i <= lednum; i++) {
    leds[i] = CRGB(r,g,b);
    FastLED.show();
    delay(40);
  }
}