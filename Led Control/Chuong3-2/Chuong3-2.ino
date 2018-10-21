#define button 2
#define rLeda 6
#define rLedb 7
#define yLed 5
#define bLeda 3
#define bLedb 4
int led[] = {3,4,5,6,7};
unsigned long triggerTime;
unsigned long waitingTime;
void whenPress()
{
  if(digitalRead(bLeda)==1){
    off();
    digitalWrite(yLed,HIGH);
  }
  else if(digitalRead(yLed)==1){
    off();
    digitalWrite(rLeda,HIGH);
    digitalWrite(rLedb,HIGH);
  }else{
    off();
    digitalWrite(bLeda,HIGH);
    digitalWrite(bLedb,HIGH);
  }
}
void notPress(){
  if(digitalRead(rLeda)==1){
    off();
    digitalWrite(yLed,HIGH);
  }
  else if(digitalRead(yLed)==1){
    off();
    digitalWrite(bLeda,HIGH);
    digitalWrite(bLedb,HIGH);
  }else{
    off();
    digitalWrite(rLeda,HIGH);
    digitalWrite(rLedb,HIGH);
  }
}
void off()
{
  for (int i = 0; i < 5; i++)
    digitalWrite(led[i], LOW);
}

void setup()
{
  Serial.begin(9600);
  pinMode(button, INPUT);
  for(int i=0;i<5;i++)
    pinMode(led[i],OUTPUT);
  digitalWrite(bLeda, HIGH);
  digitalWrite(bLedb, HIGH);
  digitalWrite(yLed, LOW);
  digitalWrite(rLeda, LOW);
  digitalWrite(rLedb, LOW);
  waitingTime = millis();
}

void loop()
{
  if (digitalRead(button) == HIGH) //nếu nhấn nút
  {
    triggerTime = millis();
    delay(50);  //chống nhiễu
    while (millis() - triggerTime < 4950)
    {
      if (digitalRead(button) == HIGH)
          ; //chờ đủ 5s
      else
        return; //nếu nhả nút trước 5s thì return
    }
    whenPress();
    waitingTime = millis();
  }
  else  //nếu không nhấn nút
  {
    if (millis() - waitingTime > 5000)
    {
      notPress();
      waitingTime = millis(); //cập nhật waitingTime
    }
  }
}
