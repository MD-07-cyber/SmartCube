#include <ESP32Servo.h>


Servo servo1;
Servo servo2;
Servo servo3;


int selectedServo = 1;


void setup() {
  servo1.attach(18);
  servo2.attach(19);
  servo3.attach(21);
}


void loop() {
  switch (selectedServo) {
    case 1:
      servo1.write(0);
      delay(250);
      servo1.write(90);
      delay(250);
      break;
    case 2:
      servo2.write(0);
      delay(250);
      servo2.write(90);
      delay(250);
      break;
    case 3:
      servo3.write(0);
      delay(250);
      servo3.write(90);
      delay(250);
      break;
  }
}