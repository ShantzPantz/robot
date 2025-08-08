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
#include "cam_controller.h"
#include "serial_helper.h"

#define BUTTON_PIN 2

bool lastButtonState = HIGH;  // Start HIGH because of pull-up
unsigned long uniqueID; 

RCInput rcInput(Serial1);
TankController tankController(rcInput);
CamController camController(rcInput);
SerialHelper esp32_vision(Serial2, "vision");
SerialHelper esp32_audio(Serial3, "audio");


void setup()
{
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Start serial monitor for debugging
  Serial.begin(115200);
  Serial1.begin(9600);
  Serial2.begin(9600);

  rcInput.setup();

  tankController.setup();
  camController.setup();

}

void handleButtonState() {
    bool buttonState = digitalRead(BUTTON_PIN);
    if (buttonState != lastButtonState) {
      // Button down
      if (buttonState == LOW) {
        Serial.println("Button pressed");
        uniqueID = millis();        
        String cmd = "START:" + String(uniqueID);
        // Send message to ESP23 cams here
        esp32_vision.writeLine(cmd);
        esp32_audio.writeLine(cmd);
      } else {
        String cmd = "END:" + String(uniqueID);
        Serial.println("Button released");
        // Send message to ESP23 cams here
        esp32_vision.writeLine(cmd);
        esp32_audio.writeLine(cmd);
      }
      lastButtonState = buttonState;
    }   
}

void loop() {
  handleButtonState();

  rcInput.loop();

  tankController.loop();
  camController.loop(); 

  // delay(20);  // keep this small for responsiveness
}

