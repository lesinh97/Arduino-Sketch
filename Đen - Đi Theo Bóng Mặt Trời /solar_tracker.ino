
#include <Servo.h>

float photo_r_0,photo_r_1,photo_r_2; // value of photoresistors
int servo1=6; // servo pin
int servo2=7;
int angle_rot=90, angle_inc=90;
Servo servo_rotate;
Servo servo_tilt;

void setup() {
  servo_rotate.attach(servo1);   // set up servo1 is a rotate servo
  servo_tilt.attach(servo2); //servo2 is a backward servo
  Serial.begin(9600);
  delay(2000);
}

void loop() {
  while ((angle_rot > 10 ) && (angle_rot< 170) && (angle_inc > 10) && (angle_inc < 170)) // make sure they won't overlap :))
  {
    photo_r_0 = analogRead(A0); Serial.print("R_0 :   "); Serial.println(photo_r_0); // 8-bit analog input = 1024 values = 0 to 5V
    photo_r_1 = analogRead(A1); Serial.print("R_1 :   "); Serial.println(photo_r_1);
    photo_r_2 = analogRead(A2); Serial.print("R_2 :   "); Serial.println(photo_r_2);

    if ( photo_r_0 < photo_r_1-3)  // depending on the value of the photoresistors, we will modulate the angle of the servomotors
    {
      angle_rot = angle_rot-1;
      servo_rotate.write(angle_rot);
    }
    else if (photo_r_0 > photo_r_1+3)
    {
      angle_rot = angle_rot+1;
      servo_rotate.write(angle_rot);
    }

    if (photo_r_2 > photo_r_1+3)
    {
      angle_inc = angle_inc-1;
      servo_tilt.write(angle_inc);
    }
    else if (photo_r_2 < photo_r_1-3)
    {
      angle_inc = angle_inc+1;
      servo_tilt.write(angle_inc);
    }
  }
  Serial.println(" SERVO ANGEL HAS FALLEN :)) ");
}
