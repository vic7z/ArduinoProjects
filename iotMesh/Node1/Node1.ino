#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 5 
#define DHTTYPE    DHT11
 
RF24 radio(7, 8);               
RF24Network network(radio);
DHT_Unified dht(DHTPIN, DHTTYPE);

      
const uint16_t this_node = 01;  
const uint16_t master00 = 00;    

const unsigned long interval = 10; 
unsigned long last_sent;            

struct MyData {
    byte id; 
    float moister; 
    float temp;
    float humidity;
    float voltage;
    bool chargingstat;
};


MyData data;


void setup() {
  SPI.begin();
  radio.begin();
  network.begin(90, this_node);  
  radio.setDataRate(RF24_2MBPS);
  dht.begin();
  data.id=1;
  data.chargingstat=true;
}

void loop() {
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    network.update();
  //===== Sending =====//
  
  unsigned long now = millis();
  if (now - last_sent >= interval) {   
    last_sent = now;
    data.moister = 72;
    data.voltage=analogRead(A2)*5.0/1024.0;
    data.temp=event.temperature;
    dht.humidity().getEvent(&event);
    data.humidity=event.relative_humidity;
    RF24NetworkHeader header(master00);   
    bool ok = network.write(header, &data, sizeof(MyData)); 
  }
}
