#include <SoftwareSerial.h>

#define trigPin1 A0
#define echoPin1 A1
#define trigPin2 A2
#define echoPin2 A3

SoftwareSerial sim(10, 11);
long duration, distance, FIRSTSensor,SECONDSensor,sumDistance;


int _timeout;
String number = "+91XXXXXX"; 

void setup() {

  Serial.begin(9600);
  Serial.println("Sistem Started...");
  sim.begin(9600);
  delay(1000);
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);

}

void loop() {
  SonarSensor(trigPin1, echoPin1);
  FIRSTSensor = distance;
  Serial.print("dis 1:" );
  Serial.println(distance);
  SonarSensor(trigPin2, echoPin2);
  SECONDSensor = distance;
  sumDistance=FIRSTSensor+SECONDSensor;
  Serial.print("dis 2:" );
  Serial.println(distance);
  if(sumDistance>150 && sumDistance<300){
    callNumber();
    delay(3000);
    SendMessage();
  }
  
}

void SendMessage() {

  sim.println("AT+CSMP=17,167,0,0");
  delay(200);
  sim.print("AT+CMGS=\"");
  sim.print(number);
  sim.println("\"");
  delay(150);
  sim.print("LandSlide Detected");
  sim.write(26);
  delay(3000);
  Serial.println("");
  Serial.print("Send SMS to:");
  Serial.print(number);
  Serial.print(":");
  Serial.println("ho");
}

void RecieveMessage() {
  Serial.println ("SIM800L Read an SMS");
  sim.println("AT+CMGF=1");
  delay(200);
  sim.println("AT+CNMI=1,2,0,0,0"); // AT Command to receive a live SMS
  delay(200);
  Serial.write ("Unread Message done");
}


void callNumber() {
  sim.print (F("ATD"));
  sim.print (number);
  sim.print (F(";\r\n"));
//  _buffer = _readSerial();
//  Serial.println(_buffer);
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
