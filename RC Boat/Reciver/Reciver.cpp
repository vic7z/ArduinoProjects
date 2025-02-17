#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Arduino.h>
#include <Wire.h>
#include <AHT20.h>
#include <Adafruit_INA219.h>

AHT20 aht20;
Adafruit_INA219 ina219;

float ahtValue = -1;

const uint64_t pipeIn = 0xE8E8F0F0E1LL;

bool radiocheck = true;

RF24 radio(7, 8);

int throttle = 0;

int leftMotorSpeed = 0;
int rightMotorSpeed = 0;

int leftmotor1 = 3;
int leftmotor2 = 4;

int rightmotor1 = 9;
int rightmotor2 = 10;

int enl = 5;
int enr = 6;

int voltage = A7;

float shuntvoltage = 0;
float busvoltage = 0;
float current_mA = 0;
float loadvoltage = 0;
float power_mW = 0;

struct txData {
  byte throttle;
  byte yaw;
  byte pitch;
  byte roll;
  byte AUX1;
  byte AUX2;
  bool state;
};

struct rxTelemetry {
  float rxVoltage;
  float current;
  float power;
  float temp;
};

txData data;

rxTelemetry rxData;

void resetData() {

  data.throttle = 0;
  data.yaw = 127;
  data.pitch = 127;
  data.roll = 127;
  data.AUX1 = 0;
  data.AUX2 = 0;
  data.state = false;
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  aht20.begin();
  ina219.begin();
  resetData();
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(1, pipeIn);
  radio.enableAckPayload();
  radio.startListening();

  pinMode(leftmotor1, OUTPUT);
  pinMode(leftmotor2, OUTPUT);
  pinMode(rightmotor2, OUTPUT);
  pinMode(rightmotor2, OUTPUT);

  pinMode(voltage, INPUT);
}

unsigned long lastRecvTime = 0;

void recvData() {
  while (radio.available()) {
    radio.read(&data, sizeof(txData));
    rxData.rxVoltage = busvoltage;
    rxData.current = current_mA;
    rxData.power = power_mW;
    rxData.temp = ahtValue;
    radio.writeAckPayload(1, &rxData, sizeof(rxData));
    lastRecvTime = millis();
  }
}
unsigned long previousMillis = 0;

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= 2000) {
    previousMillis = currentMillis;
    ahtValue = aht20.getTemperature();
    shuntvoltage = ina219.getShuntVoltage_mV();
    busvoltage = ina219.getBusVoltage_V();
    current_mA = ina219.getCurrent_mA();
    power_mW = ina219.getPower_mW();
    loadvoltage = busvoltage + (shuntvoltage / 1000);
  }


  recvData();
  unsigned long now = millis();
  if (now - lastRecvTime > 1000) {
    resetData();
    radiocheck = false;
  } else {
    radiocheck = true;
  }

  int rollValue = map(data.roll, 0, 255, -127, 127) + 1;
  leftMotorSpeed = constrain(data.throttle + rollValue, 0, 255);
  rightMotorSpeed = constrain(data.throttle - rollValue, 0, 255);

  if (data.state) {

    digitalWrite(leftmotor1, HIGH);
    digitalWrite(leftmotor2, LOW);

    digitalWrite(rightmotor1, HIGH);
    digitalWrite(rightmotor2, LOW);

    analogWrite(enl, leftMotorSpeed);
    analogWrite(enr, rightMotorSpeed);

  } else {
    digitalWrite(leftmotor1, LOW);
    digitalWrite(leftmotor2, LOW);

    digitalWrite(rightmotor1, LOW);
    digitalWrite(rightmotor2, LOW);
  }

  Serial.print("leftMotorSpeed: ");
  Serial.print(leftMotorSpeed);
  Serial.print("  ");
  Serial.print("rightMotorSpeed: ");
  Serial.print(rightMotorSpeed);
  Serial.print("  roll value : ");
  Serial.print(rollValue);
  Serial.print("   voltage ");
  Serial.print(rxData.rxVoltage);
  Serial.print("   temp ");
  Serial.print(rxData.temp);
  Serial.print("\n");
  // Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.print(" V");
  // Serial.print(" Shunt Voltage: "); Serial.print(shuntvoltage); Serial.print(" mV");
  // Serial.print(" Load Voltage:  "); Serial.print(rxData.rxVoltage); Serial.print(" V");
  // Serial.print(" Current:       "); Serial.print(current_mA); Serial.print(" mA");
  // Serial.print(" Power:         "); Serial.print(power_mW); Serial.print(" mW");
  // Serial.println("");
}