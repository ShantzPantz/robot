#ifndef TANKCONTROLLER_H
#define TANKCONTROLLER_H

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
};

#endif
