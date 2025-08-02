#include <Arduino.h>
#include "tank_controller.h"
#include "rc_input.h"

// TankController::TankController(RCInput &rc) : rcInput(rc) {
//     Serial.println("Creating TankController.");
// }

TankController::~TankController() {
    Serial.println("Destroying TankController.");
}

void TankController::setup() {
    Serial.println("Setting up TankController.");   

    // Set all the motor control pins to outputs
    pinMode(in1A, OUTPUT);
    pinMode(in2A, OUTPUT);
    pinMode(in1B, OUTPUT);
    pinMode(in2B, OUTPUT);
    pinMode(stby, OUTPUT);

    // Keep motors on standby for two seconds & flash LED
    digitalWrite(stby, LOW);
    digitalWrite(carLED, HIGH);
    delay (1000);
    digitalWrite(carLED, LOW);
    delay (1000);
    digitalWrite(stby, HIGH);
}

void TankController::loop() {
    // Read inputs    
    // int throttle = readChannel(1, -255, 255, 0);
    // int turn     = -readChannel(0, -255, 255, 0);
    int turn = rcInput.getCH1();
    int throttle = rcInput.getCH2();

    // Smoothing
    static int smoothThrottle = 0; // static here means it will only be initialized once 
    static int smoothTurn = 0;
    float alpha = 0.2;
    smoothThrottle = alpha * throttle + (1 - alpha) * smoothThrottle;
    smoothTurn     = alpha * turn     + (1 - alpha) * smoothTurn;

    // Deadzone
    int dz = 15;
    if (abs(smoothThrottle) < dz) smoothThrottle = 0;
    if (abs(smoothTurn) < dz) smoothTurn = 0;

    int motorA, motorB;

    if (smoothThrottle == 0 && abs(smoothTurn) > dz) {
        motorA = smoothTurn;
        motorB = -smoothTurn;
    } else {
        motorA = smoothThrottle + smoothTurn;
        motorB = smoothThrottle - smoothTurn;
    }

    motorA = constrain(motorA, -255, 255);
    motorB = constrain(motorB, -255, 255);

    mControlA(abs(motorA), motorA >= 0);
    mControlB(abs(motorB), motorB >= 0);
}

// Control Motor A
void TankController::mControlA(int mspeed, int mdir) {

  if (abs(mspeed) < 5) {
    digitalWrite(in1A, LOW);
    digitalWrite(in2A, LOW);
  }
  // determine direction
  else if (mdir == 0) {
    // Motor backward
    digitalWrite(in2A, LOW);
    analogWrite(in1A, mspeed);
  } else {
    // Motor forward
    digitalWrite(in1A, LOW);
    analogWrite(in2A, mspeed);
  }
}

void TankController::mControlB(int mspeed, int mdir) {
  Serial.print(mspeed);
  // see if we're idle
  if (abs(mspeed) < 5) {
    digitalWrite(in1B, LOW);
    digitalWrite(in2B, LOW);
  }
  // determine direction
  else if (mdir == 0) {
    // Motor backward
    digitalWrite(in2B, LOW);
    analogWrite(in1B, mspeed);    
  } else {
    // Motor forward
    digitalWrite(in1B, LOW);
    analogWrite(in2B, mspeed);    
  }
}