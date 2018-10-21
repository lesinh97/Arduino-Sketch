void setup() {
  pinMode(8,OUTPUT);
}
void chuS(){
    for(int i=0;i<3;i++){
      digitalWrite(8,HIGH);
      delay(100);
      digitalWrite(8,LOW);
      delay(100);
    }
  }
void chuO(){
      for(int i=0;i<3;i++){
      digitalWrite(8,HIGH);
      delay(200);
      digitalWrite(8,LOW);
      delay(100);
    }
  }
void morse(){
  chuS();
  delay(1000);
  chuO();
  delay(1000);
  chuS();
  delay(1000);
}
void loop() {
  // put your main code here, to run repeatedly:
  morse();
  delay(5000);
}
