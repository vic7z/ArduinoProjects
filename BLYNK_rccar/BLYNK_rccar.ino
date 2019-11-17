
#define BLYNK_PRINT Serial
#define l1 12
#define l2 13
#define r1 14
#define r2 15
#define servopin 3
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Servo.h>

char auth[] = "5iuuRrTKZiOPjhKU3Qt-glY5RmJ2fs45";
char ssid[] = "spaceship";
char pass[] = "88888888";

int f = 0, b = 0, r = 0, l = 0, ser = 0;

Servo servo;

BLYNK_WRITE(V1)
{
  f = param.asInt();
}

BLYNK_WRITE(V2)
{
  b = param.asInt();
}

BLYNK_WRITE(V3)
{
  l = param.asInt();
}

BLYNK_WRITE(V4)
{
  r = param.asInt();
}
BLYNK_WRITE(V5)
{
  ser = param.asInt();
}

void setup()
{
  pinMode(l1,OUTPUT);
  pinMode(r1,OUTPUT);
  pinMode(l2,OUTPUT);
  pinMode(r2,OUTPUT);
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);
  servo.attach(servopin);
}

void loop()
{
  Blynk.run();
  if (l)
  {
    lw();
    Serial.println("lw");
  }else
  {
   lo();
  }
                                                          
  if (r)
  {
    rw();
    Serial.println("rw");
  }else
  {
    lo();
  }
  
  if (f)
  {
    fd();
    Serial.println("fd");
  }else
  {
    lo();
  }

  if (b)
  {
    bw();
    Serial.println("bw");
  }else
  {
    lo();
  }

  if (ser)
  {
    servo.write(90);
  }
  else
  {
    servo.write(0);
  }
}

void fd()
{
  digitalWrite(l1, HIGH);
  digitalWrite(r1, HIGH);
  digitalWrite(l2, LOW);
  digitalWrite(r2, LOW);
}

void bw()
{
  digitalWrite(l2, HIGH);
 digitalWrite(r2, HIGH);
  digitalWrite(l1, LOW);
  digitalWrite(r1, LOW);
}

void rw()
{
 digitalWrite(l1, HIGH);
  digitalWrite(l2, LOW);
 digitalWrite(r2, HIGH);
  digitalWrite(r1, LOW);
}

void lw()
{
 digitalWrite(l2, HIGH);
  digitalWrite(l1, LOW);
 digitalWrite(r1, HIGH);
  digitalWrite(r2, LOW);
}
void lo(){
   digitalWrite(l1, LOW);
  digitalWrite(r1, LOW);
  digitalWrite(l2, LOW);
  digitalWrite(r2, LOW);
}
  
 