#include <Servo.h>
#include <Arduino.h>

Servo servo1;
Servo servo2;

void setup() {
  servo1.attach(45);  // Attach servo1 to pin 11
  servo2.attach(12);  // Attach servo2 to pin 12
}

void loop() {
  // Move both servos to 0 degrees
  servo1.write(0);
  servo2.write(0);
  delay(1000);

  // Move both servos to 90 degrees
  servo1.write(90);
  servo2.write(90);
  delay(1000);

  // Move both servos to 180 degrees
  servo1.write(180);
  servo2.write(180);
  delay(1000);
}
