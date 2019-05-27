#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#define I2C_ADDRESS 0x3C

const uint64_t pipeOut = 0xE8E8F0F0E1LL;

SSD1306AsciiWire oled;

RF24 radio(7,8);
 
uint8_t col[2];
uint8_t rows;


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




int mapJoystickValues(int val, int lower, int middle, int upper, bool reverse)
{
  val = constrain(val, lower, upper);
  if ( val < middle )
    val = map(val, lower, middle, 0, 128);
  else
    val = map(val, middle, upper, 128, 255);
  return ( reverse ? 255 - val : val );
}

void loop()
{ 
 
  data.throttle = mapJoystickValues( analogRead(A0), 13, 524, 1015, true );
  data.yaw      = mapJoystickValues( analogRead(A1),  1, 505, 1020, true );
  data.pitch    = mapJoystickValues( analogRead(A2), 12, 544, 1021, true );
  data.roll     = mapJoystickValues( analogRead(A3), 34, 522, 1020, true );
  data.AUX1     = digitalRead(9);
  data.AUX2     = digitalRead(10);

  radio.write(&data, sizeof(MyData));

  
  
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
     
    
  
}
