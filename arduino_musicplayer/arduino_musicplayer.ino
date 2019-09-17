#include "SD.h"
#include "TMRpcm.h"
#include "SPI.h"
#include <FastLED.h>    


#define SD_ChipSelectPin 4    
#define LED_PIN     7     //pin in witch RGB led is connected
#define NUM_LEDS    1     //no of RGB leds
#define pre 3             //previous music button
#define fd 6              //next music button
TMRpcm music;
int count = 1;            //button press counter
CRGB leds[NUM_LEDS];

void setup(){
  
  music.speakerPin = 9;   //audio outputr
  Serial.begin(9600);     //serial debugger
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  if (!SD.begin(SD_ChipSelectPin)) {
    Serial.println("SD fail");
    return;
    
    }
pinMode(pre,INPUT_PULLUP); 
pinMode(fd,INPUT_PULLUP);
  leds[0] = CRGB(0, 255, 0);  // setting green light (R,G,B), 0 means off ,255 means full brightness 
  FastLED.show();             //displaying greenlight

music.setVolume(5);           // setting up volume 0 - 7

}

void loop(){
  
  if(!digitalRead(fd)){
    count ++;
    delay(1000); // 1 second delay for counting
    }
  if(count>0){
     if(!digitalRead(pre)){
    count --;
     delay(1000);
    }
    else{
      count = 1;
      }
     }

  switch(count){
    case 1:
      music.play("1.wav");
       Serial.println("1");
        leds[0] = CRGB(0, 255, 0); 
        FastLED.show();       
      break;
    case 2:
       music.play("2.wav");
       Serial.println("2");
       break;
    case 3:
       music.play("3.wav");
       Serial.println("3");
       break;
   case 4:
       music.play("4.wav");
       Serial.println("4");
       break;   
    case 5:
       music.play("5.wav");
       Serial.println("5");
       break;
    default:
      leds[0] = CRGB(255, 0, 0);
      FastLED.show();
      break;
    }



     
  }
