#include <SoftwareSerial.h>
#define trigPin1 A1
#define echoPin1 A0
#define floatSensor 12


SoftwareSerial SIM900(7, 8); 
long duration, distance, Sensor;

String number = "+91XXXXXXXXX"; 
String msg="";

void setup() {
  Serial.begin(9600);
  SIM900.begin(19200);
  delay(10000);   
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(floatSensor,INPUT);
  

}

void loop() { 
  if(digitalRead(floatSensor)==LOW){
  SonarSensor(trigPin1, echoPin1);
  Sensor = distance;
  Serial.println (Sensor);
  if(Sensor>25){
    sendSMS("LOW RISk");
    Serial.println ("LOW RISk");
    callNumber();
  }else if(Sensor>=15 and Sensor<20){
     sendSMS("Moderate RISk");
     Serial.println ("Moderate RISk");
     callNumber();
  }else if(Sensor<10){
     sendSMS("High RISk");
     Serial.println (Sensor);
     callNumber();
  }
  }

}



void sendSMS(String msf) {

  SIM900.println("AT+CSMP=17,167,0,0");
  delay(200);
  SIM900.print("AT+CMGF=1\r"); 
  delay(100);

  SIM900.print("AT+CMGS=\"");
  SIM900.print(number); 
  SIM900.println("\"");
  delay(150);
 
  SIM900.println(msg); 
  delay(100);
  SIM900.println((char)26); 
  delay(100);
  SIM900.println();
  delay(5000); 
}



void callNumber() {
  SIM900.print (F("ATD"));
  SIM900.print(number);
  SIM900.print (F(";\r\n"));

}


void SonarSensor(int trigPin,int echoPin)
{
digitalWrite(trigPin, LOW);
delayMicroseconds(2);
digitalWrite(trigPin, HIGH);
delayMicroseconds(10);
digitalWrite(trigPin, LOW);
duration = pulseIn(echoPin, HIGH);
distance = (duration/2) / 29.1;
}
