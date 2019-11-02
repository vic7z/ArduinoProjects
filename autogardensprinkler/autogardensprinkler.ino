#define sensor A0
#define pumb 6
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(sensor,INPUT);
  pinMode(pumb,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
 int moisture = analogRead(sensor);
 Serial.println(moisture);

 if(moisture>750){
  analogWrite(pumb,125);
  }else{
    analogWrite(pumb,0);
    }
 
}
