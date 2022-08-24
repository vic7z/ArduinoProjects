#include <SPI.h>
#include <Wire.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"


#define I2C_ADDRESS 0x3C
const uint64_t pipeIn = 0xE8E8F0F0E1LL; 

SSD1306AsciiWire display;



RF24 radio(2,15);
uint8_t col[1]; 
uint8_t rows;   

long previousMillis = 0; 
long interval = 10000;

struct MyData {
    
    float alt;
    float preassure;
    float temp;
    float vol;
};

MyData data;



void setup()
{
    Serial.begin(9600);

    radio.begin();
    radio.setAutoAck(false);
    radio.setDataRate(RF24_250KBPS);
    radio.openReadingPipe(1,pipeIn);
    radio.startListening();
    Wire.begin();
    Wire.setClock(400000L);
    display.begin(&Adafruit128x64, I2C_ADDRESS);
    display.setFont(System5x7);
    display.clear();
    display.println("VOL  :");
    display.println("TEMP :");
    display.println("PRESS:");
    display.println("ASL  :");

  col[0] = display.fieldWidth(strlen("ADC0  : "));
  col[1] = display.fieldWidth(strlen("ADC0  :"));
  rows = display.fontRows();
  delay(3000);  
}





void recvData()
{
    while ( radio.available() ){
        radio.read(&data, sizeof(MyData));
        }
}


 
void loop()
{

    recvData();

    display.clearField(col[0%2], rows*(0/2), 4);    
    display.print(data.vol);
    display.println("V");

    display.clearField(col[2%2], rows*(2/2), 4);    
    display.print(data.temp);
    display.println("C");

    display.clearField(col[4%2], rows*(4/2), 4);    
    display.print(data.preassure);
    display.println("pa");

    display.clearField(col[6%2], rows*(6/2), 4);    
    display.print(data.alt);
    display.println("M");
    
    unsigned long currentMillis = millis();
    if(currentMillis - previousMillis > interval) {
      display.clear();
      ESP.deepSleep(0); 
    }
   
    

}
