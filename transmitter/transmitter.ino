#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

#define I2C_ADDRESS 0x3C
#define j1x A0
#define j1y A1
#define j1b 9
#define j2x A2
#define j2y A3
#define j2b 3
#define voltage A7
const uint64_t pipeOut = 0xE8E8F0F0E1LL;

SSD1306AsciiWire oled;

RF24 radio(7,8);
 
uint8_t col[2];
uint8_t rows;
String state;

struct MyData {
  byte throttle;
  byte yaw;
  byte pitch;
  byte roll;
  byte AUX1;
  byte AUX2;
};

MyData data;

void resetData() 
{
 
  data.throttle = 0;
  data.yaw = 127;
  data.pitch = 127;
  data.roll = 127;
  data.AUX1 = 0;
  data.AUX2 = 0;
}

void setup()
{
  
  radio.begin();
  Wire.begin();
  Wire.setClock(400000L);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(pipeOut);
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(System5x7);
  pinMode(j1x,INPUT);
  pinMode(voltage,INPUT);
  pinMode(j2x,INPUT);
  pinMode(j1b,INPUT_PULLUP);
  pinMode(j1y,INPUT);
  pinMode(j2y, INPUT);
  pinMode(j2b,INPUT_PULLUP);
  oled.clear();
   oled.set1X();
  oled.println("initializing...");
  
  oled.println("");
  
  oled.println("@victor_gpz");
  delay(2000);
  
  resetData();
  oled.clear();


  oled.println("throt: 255 yaw : 255");
  oled.println("pitch: 255 roll: 255");
  oled.println("Aux 1: 255 Aux2: 255");
  
    
  col[0] = oled.fieldWidth(strlen("ADC0: "));
  col[1] = oled.fieldWidth(strlen("ADC0: 9999 ADC1: "));
  rows = oled.fontRows();
  delay(3000);
 
}







void loop()
{ 
 
 float value = analogRead(voltage);
 float vo  = value /1023*5;
 

  data.throttle = map( analogRead(j1x), 0, 1023, 0, 255 );
  data.yaw      = map( analogRead(j1y),  0, 1023, 0, 255 );
  data.pitch    = map( analogRead(j2x), 0, 1023, 0, 255 );
  data.roll     = map( analogRead(j2y), 0, 1023, 0, 255 );
  data.AUX1     = digitalRead(j1b);
  data.AUX2     = digitalRead(j2b);

  radio.write(&data, sizeof(MyData));

 int per = map(data.throttle, 0, 255, 0, 100);
  
  
    oled.clearField(col[0%2], rows*(0/2), 4);    
    oled.print(data.throttle);

    oled.clearField(col[1%2], rows*(1/2), 4);    
    oled.print(data.yaw);

    oled.println();

    oled.clearField(col[2%2], rows*(2/2), 4);    
    oled.print(data.pitch);

    oled.clearField(col[3%2], rows*(3/2), 4);    
    oled.print(data.roll);

    oled.println();

    oled.clearField(col[4%2], rows*(4/2), 4);    
    oled.print(data.AUX1);

    oled.clearField(col[5%2], rows*(5/2), 4);    
    oled.print(data.AUX2);

  
    oled.println();
    oled.println();
    oled.print("voltage  :");
    oled.print(vo);
    oled.print("v");
    oled.println();
    oled.print("throttle :" );
    oled.print(per);
    oled.println(" %");
     if(data.throttle==0 && data.pitch==0){
         state="armed   ";
         
    }

    if(data.throttle==0 && data.pitch==0 && data.yaw==0 && data.roll==0){
      state="disarmed";
    }
    oled.print("motor :");
    oled.println(state);

    
}
