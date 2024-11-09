#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

#define I2C_ADDRESS 0x3C
#define j1x A0
#define j1y A1
#define j1b 9
#define j2x A2
#define j2y A3
#define j2b 3
#define voltage A7

const uint64_t pipeOut = 0xE8E8F0F0E1LL;

SSD1306AsciiWire oled;

RF24 radio(7, 8);

uint8_t col[2];
uint8_t rows;
String state;
String status;

int offsetThrottle = 0;
int offsetYaw = 0;
int offsetPitch = 0;
int offsetRoll = 0;

struct txData
{
  byte throttle;
  byte yaw;
  byte pitch;
  byte roll;
  byte AUX1;
  byte AUX2;
  bool state;
};

struct rxTelemetry
{
  byte rxVoltage;
};

txData data;
rxTelemetry rxData;

void resetData()
{

  data.throttle = 0;
  data.yaw = 127;
  data.pitch = 127;
  data.roll = 127;
  data.AUX1 = 0;
  data.AUX2 = 0;
  data.state = false;
}

void setup()
{

  radio.begin();
  Wire.begin();
  Serial.begin(9600);

  Wire.setClock(400000L);

  radio.setDataRate(RF24_250KBPS);
  radio.enableAckPayload();
  radio.setRetries(10, 5);
  radio.openWritingPipe(pipeOut);

  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(System5x7);
  oled.clear();
  oled.set1X();
  pinMode(j1x, INPUT);
  pinMode(voltage, INPUT);
  pinMode(j2x, INPUT);
  pinMode(j1b, INPUT_PULLUP);
  pinMode(j1y, INPUT);
  pinMode(j2y, INPUT);
  pinMode(j2b, INPUT_PULLUP);

  oled.println("initializing...");
  oled.println("");
  oled.println("@vic7z");

  delay(1000);
  calibrateJoysticks();

  resetData();
  oled.clear();

  oled.println("throt: 255 yaw : 255");
  oled.println("pitch: 255 roll: 255");
  oled.println("Aux 1: 255 Aux2: 255");

  col[0] = oled.fieldWidth(strlen("ADC0: "));
  col[1] = oled.fieldWidth(strlen("ADC0: 9999 ADC1: "));
  rows = oled.fontRows();
  delay(3000);
}

void calibrateJoysticks()
{
  oled.clear();
  oled.print("Calibrating Joysticks");
  int initialThrottle = analogRead(j1x);
  int initialYaw = analogRead(j1y);
  int initialPitch = analogRead(j2x);
  int initialRoll = analogRead(j2y);

  offsetThrottle = map(initialThrottle, 0, 1023, 0, 255) - 127;
  offsetYaw = map(initialYaw, 0, 1023, 0, 255) - 127;
  offsetPitch = map(initialPitch, 0, 1023, 0, 255) - 127;
  offsetRoll = map(initialRoll, 0, 1023, 0, 255) - 127;
  delay(500);
  oled.clear();
  oled.println("Offset values set");
  oled.println(" ");
  oled.println(" ");
  oled.print("throttle :");
  oled.println(offsetThrottle);
  oled.print("yaw :");
  oled.println(offsetYaw);
  oled.print("pitch :");
  oled.println(offsetPitch);
  oled.print("roll :");
  oled.println(offsetRoll);
  delay(3000);
  oled.clear();
}

void loop()
{
  float value = analogRead(voltage);
  float vo = value * (3.3 / 1023.0);

  data.throttle = constrain(map(analogRead(j1x), 0, 1023, 0, 255) - offsetThrottle, 0, 255);
  data.yaw = constrain(map(analogRead(j1y), 0, 1023, 0, 255) - offsetYaw, 0, 255);
  data.pitch = constrain(map(analogRead(j2x), 0, 1023, 0, 255) - offsetPitch, 0, 255);
  data.roll = constrain(map(analogRead(j2y), 0, 1023, 0, 255) - offsetRoll, 0, 255);
  data.AUX1 = digitalRead(j1b);
  data.AUX2 = digitalRead(j2b);
  data.AUX1 = digitalRead(j1b);
  data.AUX2 = digitalRead(j2b);

  if (data.throttle == 255 && data.pitch == 255)
  {
    state = "armed   ";
    data.state = true;
  }

  if (data.throttle == 255 && data.pitch == 255 && data.yaw == 255 && data.roll == 255)
  {
    state = "disarmed";
    data.state = false;
  }

  status = radio.write(&data, sizeof(txData)) ? "ok" : "failed";
  if (status == "ok")
  {
    if (radio.isAckPayloadAvailable())
    {
      radio.read(&rxData, sizeof(rxData));
    }
    else
    {
      Serial.println("Acknowledge but no data ");
    }
  }

  int per = map(data.throttle, 0, 255, 0, 100);

  oled.clearField(col[0 % 2], rows * (0 / 2), 4);
  oled.print(data.throttle);

  oled.clearField(col[1 % 2], rows * (1 / 2), 4);
  oled.print(data.yaw);

  oled.println();

  oled.clearField(col[2 % 2], rows * (2 / 2), 4);
  oled.print(data.pitch);

  oled.clearField(col[3 % 2], rows * (3 / 2), 4);
  oled.print(data.roll);

  oled.println();

  oled.clearField(col[4 % 2], rows * (4 / 2), 4);
  oled.print(data.AUX1);

  oled.clearField(col[5 % 2], rows * (5 / 2), 4);
  oled.print(data.AUX2);

  oled.println();
  oled.print("TX status:");
  oled.print(status);
  oled.println();
  oled.print("voltage  :");
  oled.print(vo);
  oled.print("v");
  oled.println();
  oled.print("throttle :");
  oled.print(per);
  oled.print(" ");
  oled.println("%");
  oled.print("motor :");
  oled.println(state);
  Serial.println(rxData.rxVoltage);
}