#define BLYNK_PRINT Serial
#define BLYNK_PRINT Serial
#define DHTPIN 0          
#define DHTTYPE DHT11     
#define motor 4
#define fan 12
#define sens 5
#define soil_moisture A0
#define ser 14

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <Servo.h>


int fan_on_off =0;
int pumb =0;
int soil;
int ir;
float t;
char auth[] = "5iuuRrTKZiOPjhKU3Qt-glY5RmJ2fs45";


char ssid[] = "spaceship";
char pass[] = "88888888";

#define DHTPIN 0          // D3


#define DHTTYPE DHT11     

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
  Blynk.virtualWrite(V7,soil);
}


BLYNK_WRITE(V1){

  fan_on_off =param.asInt();
  
  }


// BLYNK_WRITE(V2){

//   pumb =param.asInt();
  
//   }


//void water(){
//  
//   soil = analogRead(soil_moisture);
//
//   if(pumb){
//
//      if(soil<400){
//          Serial.println("motor on");
//          analogWrite(motor,127);
//        
//        }
//    }   
//
//
//
//  }



void fann(){

        Serial.println("fan on");
        
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







void setup()
{
  // Debug console
  Serial.begin(9600);

  Blynk.begin(auth, ssid, pass);
  
  servo.attach(ser);
  dht.begin();

  
  timer.setInterval(1000L, sendSensor);
  pinMode(sens,INPUT);
  pinMode(pumb,OUTPUT);
  pinMode(soil_moisture,INPUT);
  pinMode(fan,OUTPUT);
}

void loop()
{
  Blynk.run();
  timer.run();

  ir = digitalRead(sens);
Serial.println(ir);
    if(!ir){
      servo.write(180);
        }else{
          servo.write(0);
          }

if(soil<100){
          Serial.println("motor on");
          analogWrite(motor,125);
        
        }else{
           analogWrite(motor,0);
          }



    
//     if(pumb){
// //      water();
//        if(soil<100){
//           Serial.println("motor on");
//           analogWrite(motor,127);
        
//         }


//       }
      
    if(fan_on_off){
      fann();
      }
   
  
}
