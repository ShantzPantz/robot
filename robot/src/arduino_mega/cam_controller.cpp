#include <Arduino.h>
#include "cam_controller.h"
#include "rc_input.h"

CamController::~CamController() {
    Serial.println("Destroying CamController.");
}

void CamController::setup() {
    Serial.println("Setting up CamController.");   
    panServo_.attach(PAN_PIN);
    tiltServo_.attach(TILT_PIN);

    panServo_.write(START_PAN);
    tiltServo_.write(START_TILT);
    delay(300);
    panServo_.detach();
    tiltServo_.detach();
}

void CamController::loop() {
    int rawPan = constrain(rcInput.getVRA(), MIN_PAN, MAX_PAN);
    int rawTilt = constrain(rcInput.getVRB(), MIN_TILT, MAX_TILT);

    static float smoothPan = START_PAN;
    static float smoothTilt = START_TILT;

    unsigned long now = millis();
  if (now - lastUpdate >= UPDATE_INTERVAL) {
    lastUpdate = now;

    // Apply exponential smoothing
    smoothPan = ALPHA * rawPan + (1 - ALPHA) * smoothPan;
    smoothTilt = ALPHA * rawTilt + (1 - ALPHA) * smoothTilt;

    // PAN
    if (abs(smoothPan - rawPan) > DETACH_THRESHOLD) {
      if (!panAttached_) {
        panServo_.attach(PAN_PIN);
        panAttached_ = true;
      }
      panServo_.write((int)smoothPan);
    } else if (panAttached_) {
      panServo_.detach();
      panAttached_ = false;
    }

    // TILT
    if (abs(smoothTilt - rawTilt) > DETACH_THRESHOLD) {
      if (!tiltAttached_) {
        tiltServo_.attach(TILT_PIN);
        tiltAttached_ = true;
      }
      tiltServo_.write((int)smoothTilt);
    } else if (tiltAttached_) {
      tiltServo_.detach();
      tiltAttached_ = false;
    }
  }
}