#include <FastLED.h>

#define LED_PIN     5
#define NUM_LEDS    30

CRGB leds[NUM_LEDS];

void setup() {

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);

}

void loop() {

  for (int i = 0; i <= 30; i++) {
    leds[i] = CRGB(0,0,125);
    FastLED.show();
    delay(40);
  }
  for (int i = 30; i >= 0; i--) {
    leds[i] = CRGB(0,125,0);
    FastLED.show();
    delay(40);
  }
  for (int i = 0; i <= 30; i++) {
    leds[i] = CRGB(125,0,0);
    FastLED.show();
    delay(40);
  }
  for (int i = 30; i >= 0; i--) {
    leds[i] = CRGB(100,125,50);
    FastLED.show();
    delay(40);
  }
  for (int i = 0; i <= 30; i++) {
    leds[i] = CRGB(255,20,147);
    FastLED.show();
    delay(40);
  }
  for (int i = 30; i >= 0; i--) {
    leds[i] = CRGB(0,206,209);
    FastLED.show();
    delay(40);
  }
  for (int i = 0; i <= 30; i++) {
    leds[i] = CRGB(25,25,112);
    FastLED.show();
    delay(40);
  }
   for (int i = 30; i >= 0; i--) {
    leds[i] = CRGB(255,69,0);
    FastLED.show();
    delay(40);
  }
   for (int i = 30; i >= 0; i--) {
    leds[i] = CRGB(255,255,2550);
    FastLED.show();
    delay(40);
  }
  }
  
