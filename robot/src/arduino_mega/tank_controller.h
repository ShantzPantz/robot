#ifndef TANKCONTROLLER_H
#define TANKCONTROLLER_H

#include <Servo.h>
#include "rc_input.h"

// LED Connection
#define carLED 13

// Motor A Control Connections
// #define pwmA 3
#define in1A 5
#define in2A 3

// Motor B Control Connections
// #define pwmB 9
#define in1B 9
#define in2B 10

// TB6612FNG Standby Pin
#define stby 6

// Servo pins for Cam / Turret 
#define servoA 11
#define servoB 12

class TankController {
public:
    TankController(RCInput &rc) : rcInput(rc) {};
    ~TankController();

    void setup();
    void loop();


private:
    RCInput &rcInput;

    // Motors
    int motorSpeedA_ = 0;
    int motorSpeedB_ = 0;
    int motorDirA_ = 1;
    int motorDirB_ = 1;

    void mControlA(int mspeed, int mdir);
    void mControlB(int mspeed, int mdir);

    // Servos
    Servo servoPan_;
    Servo servoTilt_;

    int targetPan_ = 90;
    int targetTilt_ = 90;
    int lastPan_ = 124;
    int lastTilt_ = 140;

    unsigned long lastServoUpdate_ = 0;
    const unsigned long servoUpdateInterval_ = 20;// time between steps
    unsigned long panSettleTime_ = 0;
    unsigned long tiltSettleTime_ = 0;
    const unsigned long settleDelay_ = 200; // ms to wait before detaching
    bool panAttached_ = false;
    bool tiltAttached_ = false;

    void servoUpdate();
};

#endif
