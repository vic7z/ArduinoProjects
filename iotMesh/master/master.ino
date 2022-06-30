#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

#define button 2
#define led 3

RF24 radio(7, 8);               
RF24Network network(radio);      
const uint16_t this_node = 00;   
const uint16_t node01 = 01;      
const uint16_t node012 = 012;
const uint16_t node022 = 022;

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

}

void loop() {
  network.update();
 
  while ( network.available() ) { 
    RF24NetworkHeader header;
    network.read(header, &data, sizeof(MyData)); 
    Serial.print("ID: "); Serial.print(data.id);  Serial.println("    ");
    Serial.print("moist: ");    Serial.print(data.moister);       Serial.println("    "); 
     Serial.print("temp: ");    Serial.print(data.temp);       Serial.println("    ");
    Serial.print("humidity: ");    Serial.print(data.humidity);     Serial.println("    ");
    Serial.print("voltage: ");     Serial.print(data.voltage);      Serial.println("    ");
   
  }
//  //===== Sending =====//
//  // Servo control at Node 01
//  unsigned long potValue = analogRead(A0);
//  unsigned long angleValue = map(potValue, 0, 1023, 0, 180); // Suitable for servo control
//  RF24NetworkHeader header2(node01);     // (Address where the data is going)
//  bool ok = network.write(header2, &angleValue, sizeof(angleValue)); // Send the data
//
//  // LED Control at Node 012
//  unsigned long buttonState = digitalRead(button);
//  RF24NetworkHeader header4(node012);    // (Address where the data is going)
//  bool ok3 = network.write(header4, &buttonState, sizeof(buttonState)); // Send the data
//
//  // LEDs control at Node 022
//  unsigned long pot2Value = analogRead(A1);
//  RF24NetworkHeader header3(node022);    // (Address where the data is going)
//  bool ok2 = network.write(header3, &pot2Value, sizeof(pot2Value)); // Send the data
}
