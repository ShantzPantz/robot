#ifndef CAMCONTROLLER_H
#define CAMCONTROLLER_H

#include <Servo.h>
#include "rc_input.h"

// Servo pins for Cam / Turret 
#define PAN_PIN 11
#define TILT_PIN 12

class CamController {
public:
    CamController(RCInput &rc) : rcInput(rc) {};
    ~CamController();

    void setup();
    void loop();

private:
    RCInput &rcInput;

    // Servos
    Servo panServo_;
    Servo tiltServo_;
    bool panAttached_ = false;
    bool tiltAttached_ = false;

    const float ALPHA = 0.2;
    const int DETACH_THRESHOLD = 2;
    const unsigned long UPDATE_INTERVAL = 15;
    const int START_PAN = 92;
    const int START_TILT = 120;
    const int MIN_PAN = 0;
    const int MAX_PAN = 180;
    const int MIN_TILT = 65;
    const int MAX_TILT = 130;
    unsigned long lastUpdate = 0;
};
#endif