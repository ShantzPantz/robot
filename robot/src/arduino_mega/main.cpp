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

RCInput rcInput(Serial1);
TankController tankController(rcInput);
CamController camController(rcInput);
SerialHelper esp32_vision(Serial1, "vision");
SerialHelper esp32_audio(Serial2, "audio");

void setup()
{
  // Start serial monitor for debugging
  Serial.begin(115200);

  rcInput.setup();

  tankController.setup();
  camController.setup();

}

void loop() {
  rcInput.loop();

  tankController.loop();
  camController.loop(); 

  // delay(20);  // keep this small for responsiveness
}