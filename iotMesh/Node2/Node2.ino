#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

RF24 radio(7, 8);               
RF24Network network(radio);      
const uint16_t this_node = 02;   
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
  Serial.begin(9600);
  data.id=2;
  data.chargingstat=true;
}

void loop() {
  Serial.println(analogRead(A0));
  network.update();
  //===== Sending =====//
  unsigned long now = millis();
  if (now - last_sent >= interval) {   
    last_sent = now;
    data.moister = analogRead(A0);
    data.voltage=analogRead(A2)*5.0/1024.0;
    RF24NetworkHeader header(master00);  
    bool ok = network.write(header, &data, sizeof(MyData)); 
  }
}
