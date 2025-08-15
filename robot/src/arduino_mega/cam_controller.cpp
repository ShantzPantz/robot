#include <Arduino.h>
#include "cam_controller.h"
#include "rc_input.h"

// Constants
#define DEADZONE      1
#define DETACH_DELAY  1000

CamController::~CamController() {
    Serial.println("Destroying CamController.");
}

void CamController::safeDetach(Servo &servo, int pin) {
  servo.detach();
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  Serial.print("Detatching..");
}

void CamController::setup() {
    Serial.println("Setting up CamController.");   
    panServo_.attach(PAN_PIN);
    tiltServo_.attach(TILT_PIN);

    panServo_.write(START_PAN);
    tiltServo_.write(START_TILT);
    delay(300);
    safeDetach(panServo_, PAN_PIN);
    safeDetach(tiltServo_, TILT_PIN);
    panAttached_ = false;
    tiltAttached_ = false;
}

void CamController::loop() {
    int panSpeed = rcInput.getCH4(); 
    int tiltValue = rcInput.readChannel(2, MIN_TILT, MAX_TILT, START_TILT);  

    static int panValue = START_PAN;
    static int prevTiltValue = tiltValue;
    static long lastUpdate = 0;
    static long lastPanWrite = 0;
    static long lastTiltWrite = 0;
    
    long now = millis();
    if(now - lastUpdate >= 20) {
      lastUpdate = now;

      if (abs(panSpeed) > DEADZONE) {
        if(!panAttached_) {
          panServo_.attach(PAN_PIN);
          panAttached_ = true;
          Serial.println("Attaching Pan");
        }
        panValue = constrain(panValue - constrain(panSpeed, -5, 5), MIN_PAN, MAX_PAN);              
        panServo_.write(panValue);
        lastPanWrite = now;
      } else if(panAttached_ && (now - lastPanWrite > DETACH_DELAY)) {
        safeDetach(panServo_, PAN_PIN);
        panAttached_ = false;        
      }
      
      if(abs(prevTiltValue - tiltValue) > DEADZONE) {
        if(!tiltAttached_) {
          tiltServo_.attach(TILT_PIN);
          tiltAttached_ = true;
          Serial.println("Attaching Tilt");
        }
        tiltServo_.write(tiltValue);
        prevTiltValue = tiltValue;
               
      } else if(tiltAttached_ && (now - lastTiltWrite > DETACH_DELAY)) {
        safeDetach(tiltServo_, TILT_PIN);
        tiltAttached_ = false;        
        lastTiltWrite = now;
      }     
    }

#ifdef DEBUG
    static long lastPrint = 0;
    if(millis() - lastPrint > 2000) {
      lastPrint = millis();
      Serial.println("---- DEBUG PAN TILT");
      Serial.print("pan: ");
      Serial.print(panValue);
      Serial.print("  |  tilt: ");
      Serial.print(tiltValue);      
      Serial.print("pan Speed: ");
      Serial.println(panSpeed);  
    }
    
#endif
   
}