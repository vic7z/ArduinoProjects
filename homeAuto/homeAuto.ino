
#include <SoftwareSerial.h>
SoftwareSerial DebugSerial(2, 3); // RX, TX

#define BLYNK_PRINT DebugSerial
#include <BlynkSimpleStream.h>

#include <SimpleTimer.h>
#include <DHT.h>

#define DHTPIN 5          
#define DHTTYPE DHT11     
#define motor 4
#define fan 12
#define sens 9
#define soil_moisture A0

int fan_on_off =0;
int pumb =0;

DHT dht(DHTPIN, DHTTYPE);
SimpleTimer timer;
int soil;

char auth[] = "YourAuthToken";


void sendSensor()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature(); 
   soil = analogRead(A0);
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  Blynk.virtualWrite(V5, h);
  Blynk.virtualWrite(V6, t);
  Blynk.virtualWrite(V7,soil);
}

void setup()
{
  
  DebugSerial.begin(9600);

  
  Serial.begin(9600);
  Blynk.begin(Serial, auth);
  timer.setInterval(1000L, sendSensor);
}

void loop()
{
  Blynk.run();
  timer.run(); 

  if(soil<900){
    
    analogWrite(15,127);
    digitalWrite(14,LOW);  
    }
    else if(soil >900){
       digitalWrite(15,LOW);  
        digitalWrite(14,LOW);  
      }

  
}
