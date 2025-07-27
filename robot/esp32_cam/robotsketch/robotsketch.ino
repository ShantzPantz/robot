#include <WiFi.h>
#include <HTTPClient.h>
#include "esp_camera.h"

#include "vision.h"

// Replace with your network credentials
const char* ssid = "shantz";
const char* password = "holycow11";

// Replace with your server URL
const char* serverUrl = "http://shantz-ubuntu:5000/communicate";

Vision camera;

void setup() {
  Serial.begin(115200);
  delay(100); // Small delay to let serial settle

  Serial.println("Initialize vision module.");
  
  camera.initialize();
}

void loop() {
  delay(5000);
  Serial.println("Loop.");
  camera_fb_t* fb = camera.captureFrame();
  camera.cleanup(fb);
  // takeAndUploadPhoto();
}
