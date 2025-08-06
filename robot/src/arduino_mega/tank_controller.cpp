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
    // Set Servos
    // servoPan_.attach(servoA);
    // servoTilt_.attach(servoB);

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
    delay (1000);

    servoPan_.write(lastPan_);
    servoTilt_.write(lastTilt_);
    delay(500);
}

void TankController::loop() {
    // Read inputs    
    int turn = rcInput.getCH1();
    int throttle = rcInput.getCH2();
    int panValue = rcInput.getVRA();
    int tiltValue = rcInput.getVRB(); 

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
    
    targetPan_ = panValue;
    targetTilt_ = tiltValue;

    servoUpdate();
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

void TankController::servoUpdate() {
  unsigned long now = millis();
  if (now - lastServoUpdate_ < servoUpdateInterval_) return;
  lastServoUpdate_ = now;

  // --- PAN ---
  if (lastPan_ != targetPan_) {
    if (!panAttached_) {
      servoPan_.attach(servoA);
      panAttached_ = true;
    }

    if (abs(targetPan_ - lastPan_) <= 1) {
      lastPan_ = targetPan_;
      panSettleTime_ = now; // Start the settle timer
    } else {
      lastPan_ += (targetPan_ > lastPan_) ? 2 : -2;
    }

    servoPan_.write(lastPan_);
  } else if (panAttached_ && (now - panSettleTime_ > settleDelay_)) {
    servoPan_.detach();
    panAttached_ = false;
  }

  // --- TILT ---
  if (lastTilt_ != targetTilt_) {
    if (!tiltAttached_) {
      servoTilt_.attach(servoB);
      tiltAttached_ = true;
    }

    if (abs(targetTilt_ - lastTilt_) <= 1) {
      lastTilt_ = targetTilt_;
      tiltSettleTime_ = now;
    } else {
      lastTilt_ += (targetTilt_ > lastTilt_) ? 2 : -2;
    }

    servoTilt_.write(lastTilt_);
  } else if (tiltAttached_ && (now - tiltSettleTime_ > settleDelay_)) {
    servoTilt_.detach();
    tiltAttached_ = false;
  }
}
