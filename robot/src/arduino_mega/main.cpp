/*
  Uses Flysky FS-I6X RC receiver & FS-I6X 6-ch Controller
  Uses TB6612FNG H-Bridge Motor Controller
  Drives two DC Motors

  Right stick controls direction and speed (CH1 & CH2)

  Channel functions by Ricardo Paiva - https://gist.github.com/werneckpaiva/

  Inspired by DroneBot Workshop 2021 https://dronebotworkshop.com
*/
#include <Arduino.h>
#include "rc_input.h"
#include "tank_controller.h"

RCInput rcInput(Serial1);
TankController tankController(rcInput);

void setup()
{
  // Start serial monitor for debugging
  Serial.begin(115200);
  rcInput.setup();

  // Set all the motor control pins to outputs
  pinMode(in1A, OUTPUT);
  pinMode(in2A, OUTPUT);
  pinMode(in1B, OUTPUT);
  pinMode(in2B, OUTPUT);
  pinMode(stby, OUTPUT);

  // Set LED pin as output
  // pinMode(carLED, OUTPUT);

  // Keep motors on standby for two seconds & flash LED
  digitalWrite(stby, LOW);
  digitalWrite(carLED, HIGH);
  delay (1000);
  digitalWrite(carLED, LOW);
  delay (1000);
  digitalWrite(stby, HIGH);

}

void loop() {
  rcInput.loop();

  tankController.loop(); 

  delay(20);  // keep this small for responsiveness
}