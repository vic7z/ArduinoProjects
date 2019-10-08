#define BLYNK_PRINT Serial
#define BLYNK_PRINT Serial
#define DHTPIN 0          
#define DHTTYPE DHT11     
#define motor 4
#define fan 12
#define sens 5-

#define soil_moisture A0
#define ser 14
#define DHTPIN 0          // D3
#define DHTTYPE DHT11     



#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <Servo.h>





int fan_on_off =0;
int garage =0;
int soil;

float t;

char auth[] = "5iuuRrTKZiOPjhKU3Qt-glY5RmJ2fs45";
char ssid[] = "spaceship";
char pass[] = "88888888";


DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;
Servo servo;



void sendSensor()
{

  float h = dht.readHumidity();
   t = dht.readTemperature(); 
   soil = analogRead(A0);
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

 
  Blynk.virtualWrite(V5, t);
   Blynk.virtualWrite(V6, h);
 
}





BLYNK_WRITE(V1){
  fan_on_off =param.asInt();
  }

BLYNK_WRITE(V2){
  garage =param.asInt();
  }




void setup()
{
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);
  servo.attach(ser);
  dht.begin();
  timer.setInterval(1000L, sendSensor);
  pinMode(motor,OUTPUT);
  pinMode(soil_moisture,INPUT);
  pinMode(fan,OUTPUT);
  servo.write(0);
}



void loop()
{

  Blynk.run();
  timer.run();

if(garage){
  servo.write(180);
  Serial.println("garage open");
} 
  else{
    servo.write(0);
     Serial.println("garage close");
    }


if(soil<100){

          Serial.println("motor on");
          analogWrite(motor,200);
        }else{
           analogWrite(motor,0);
          }
      Serial.println(fan_on_off);                       
   analogWrite(fan,fan_on_off);

}
