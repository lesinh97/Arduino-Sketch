byte pin[] = {4,5,6,7,8,9,10,11,12,13};

void setup() {
  Serial.begin(9600); 
  for (int x = 0; x<10; x++)
  {
    pinMode(pin[x], OUTPUT);
  }
}

void loop() {
  /* Bai 3
  for (int y = 0; y<10; y++)
  {
    digitalWrite(pin[y], HIGH);
    delay(500);
  }
  for (int x =9; x>=0; x--) 
  {
    digitalWrite(pin[x], LOW);
    delay(500);
  }
  */

  /* Bai 4
   int value = analogRead(A0);   //đọc giá trị điện áp ở chân A0 
                                //(value luôn nằm trong khoảng 0-1023)
  Serial.println(value);        //xuất ra giá trị vừa đọc
 
  Serial.println();     //xuống hàng
  delay(200);           //đợi 0.2 giây

  for (int y = 0; y<10; y++)
  {
    digitalWrite(pin[y], HIGH);
    delay(value);
  }
  for (int x =9; x>=0; x--) 
  {
    digitalWrite(pin[x], LOW);
    delay(value);
  }
  */

  /* Bai 5
    for (int x = 0; x<10; x++)
    {
      digitalWrite(pin[x], HIGH);
      digitalWrite(pin[9-x], HIGH);
      delay(100);
    }
    for (int i =4; i>=0; i--){
      digitalWrite(pin[i], LOW);
      digitalWrite(pin[9-i], LOW);
      delay(100);
    }

   */

   /* Bai 6
   int temp = 1000;
   for (int x = 0; x<5; x++) {
    temp-= 200;
    Serial.println(temp);
    digitalWrite(pin[x], HIGH);
    delay(temp);
   }
   for (int x =5; x<10; x++) {
    temp+=200;
    Serial.println(temp);
    digitalWrite(pin[x], HIGH);
    delay(temp);
   }
     for (int x =9; x>=0; x--) 
  {
    digitalWrite(pin[x], LOW);
  }
  Serial.println("End");
  */
  morse();
  delay(5000);
  
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
