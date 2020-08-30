#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

const uint64_t pipeIn = 0xE8E8F0F0E1LL; 

#define thro 9
#define ya 5
#define pit 6
#define rl 3
#define ser 4


bool radiocheck=true;

RF24 radio(7,8);

int throttle = 0;

Servo ESC;
Servo servo;
                                                                                                                                                                                                                                                                                                                         
struct MyData {
    byte throttle; 
    byte yaw;
    byte pitch;
    byte roll;
    byte AUX1;
    byte AUX2;
    bool state;
};

MyData data;

void resetData(){

    data.throttle = 0;
    data.yaw = 127;
    data.pitch = 127;
    data.roll = 127;
    data.AUX1 = 0;
    data.AUX2 = 0;
    data.state=false;

}


void setup(){
    Serial.begin(9600);

    resetData();
    radio.begin();
    radio.setAutoAck(false);
    radio.setDataRate(RF24_250KBPS);
    radio.openReadingPipe(1,pipeIn);
    radio.startListening();


}



unsigned long lastRecvTime = 0;

void recvData(){
    while ( radio.available() ){
        radio.read(&data, sizeof(MyData));
        lastRecvTime = millis();
    }
}


void loop(){
    recvData();
    unsigned long now = millis();

    if ( now - lastRecvTime > 1000 ) {

            resetData();
            radiocheck=false;
    }else{
        radiocheck=true;
    }
    

    Serial.print("Throttle: "); Serial.print(data.throttle);  Serial.print("    ");
    Serial.print("Yaw: ");      Serial.print(data.yaw);       Serial.print("    ");
    Serial.print("Pitch: ");    Serial.print(data.pitch);     Serial.print("    ");
    Serial.print("Roll: ");     Serial.print(data.roll);      Serial.print("    ");
    Serial.print("Aux1: ");     Serial.print(data.AUX1);      Serial.print("    ");
    Serial.print("Aux2: ");     Serial.print(data.AUX2);      Serial.print("\n");


    analogWrite(ya,data.yaw);
    analogWrite(pit,data.pitch);
    analogWrite(rl,data.roll);
    int rll =map(data.roll,0,255,0,180);
    servo.write(rll);

    
    if(data.state && radiocheck){
       analogWrite(thro,data.throttle);
     }else{
        analogWrite(thro,0);
    }
}