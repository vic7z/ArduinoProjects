#define BLYNK_PRINT Serial
#define l1 4
#define l2 5
#define ecg A0

#include <Wire.h>
#include "MAX30105.h"           //MAX3010x library
#include "heartRate.h"          //Heart rate calculating algorithm
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

MAX30105 particleSensor;

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred
float beatsPerMinute;
int beatAvg;
int egcvalue;
byte iskid = 0;

char auth[] = "YourAuthToken";
char ssid[] = "YourNetworkName";
char pass[] = "YourPassword";

void setup() {  
  delay(3000);
  // Initialize sensor
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);
  pinMode(l1,INPUT);
  pinMode(l2,INPUT);
  
  particleSensor.begin(Wire, I2C_SPEED_FAST); //Use default I2C port, 400kHz speed
  particleSensor.setup();                     //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running

}

void loop() {
  Blynk.run();
 long irValue = particleSensor.getIR();    //Reading the IR value it will permit us to know if there's a finger on the sensor or not
 if((digitalRead(l1) == 1)||(digitalRead(l2) == 1)){
Serial.println('!');
}
else{
  egcvalue = analogRead(ecg);

}
if(irValue > 7000){                                           //If a finger is detected
    Serial.println("BPM");             
    Serial.println(beatAvg); 
    
  if (checkForBeat(irValue) == true)                        //If a heart beat is detected
  {
    Serial.println("BPM");             
    Serial.println(beatAvg); 
    tone(3,1000);                                        //And tone the buzzer for a 100ms you can reduce it it will be better
    delay(100);
    noTone(3);                                           //Deactivate the buzzer to have the effect of a "bip"
    long delta = millis() - lastBeat;                    //Measure duration between two beats
    lastBeat = millis(); 

    beatsPerMinute = 60 / (delta / 1000.0);            //Calculating the BPM
  
    if (beatsPerMinute < 255 && beatsPerMinute > 20)               //To calculate the average we strore some values (4) then do some math to calculate the average
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
    //adult heart beat
    if(iskid == 0){
      if(beatAvg>=100 || beatAvg <=60){
        //code to send sos 
      }
    }
    //kids heart beat 
    if(iskid == 1){
      if(beatAvg>=110 || beatAvg <=70){
        //code to send sos 
        Blynk.notify("Patient is in critical condition")
      }
    }
    //infant heart beat
    if(iskid == 2){
      if(beatAvg>=120 || beatAvg<=160){
        //code to send sos
      }
    }
    } 
  }

}


if (irValue < 7000){       
     beatAvg=0;
     Serial.println("Please Place "); 
     Serial.println("your finger ");  
     noTone(3);
     }

}