#define ir 2
#define led 3
void setup() {
  Serial.begin(9600);
  pinMode(ir,INPUT);
  pinMode(led,OUTPUT);
}

void loop() {
 
Serial.println(digitalRead(ir));
  
  if(!digitalRead(ir)){
    
    digitalWrite(led,LOW);
    Serial.println("street light on");
    delay(5000);
    }else{
      
      digitalWrite(led,HIGH);
        Serial.println("street light off");
      }
}
