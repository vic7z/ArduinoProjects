#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>

#define AnalogIn A0

Adafruit_BMP280 bmp;

float adc_vol=0.0;
float in_vol=0.0;

float R1=30000.0;
float R2=7500.0;

float ref_vol=5.0;
int adc_val=0;

const uint64_t pipeOut = 0xE8E8F0F0E1LL;


RF24 radio(7,8);
 


struct MyData {
    float alt;
    float preassure;
    float temp;
    float vol;
};

MyData data;



void setup(){
  0
  radio.begin();
  bmp.begin(0x76);

  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(pipeOut);
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     
                  Adafruit_BMP280::SAMPLING_X2,     
                  Adafruit_BMP280::SAMPLING_X16,    
                  Adafruit_BMP280::FILTER_X16,      
                  Adafruit_BMP280::STANDBY_MS_500);



  delay(3000);
 
}







void loop(){ 
 
    adc_val=analogRead(AnalogIn);
    adc_vol=(adc_val*ref_vol)/1024.0;
    in_vol=adc_vol/(R2/(R1+R2));
 

  data.alt = bmp.readAltitude(1019.66);
  data.preassure =bmp.readPressure()/100;
  data.temp    = bmp.readTemperature();
  data.vol     = in_vol;

  radio.write(&data, sizeof(MyData));


  
  

    
}
