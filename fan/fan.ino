#include <DHT.h>
#define DHTPIN 3
#define DHTTYPE DHT11
#define fan 6
 
DHT dht(DHTPIN, DHTTYPE);
 
void setup(){
 
  Serial.begin(9600);
  dht.begin();
  
}
 
void loop(){
   float h = dht.readHumidity();
   float t = dht.readTemperature(); 
 
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
  }
  Serial.println(t);
  
  if(t>=31.0){
    analogWrite(fan,125);
    
  }
  if(t>=34.0){
    analogWrite(fan,255);
  }
  else{
    analogWrite(fan,75);
  }
}
