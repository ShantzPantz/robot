#include "config.h"

#include <Arduino.h>
#include <network_manager.h>
#include <vision.h>

long lastMsg = 0;
int value = 0;
const char* serverUrl = "http://192.168.2.64:5000/test_image_upload";

Vision camera;
NetworkManager networkManager;

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n--- Starting esp32cam_vision ---");

  // --- Initialize Camera --- 
  camera.initialize();

  // --- Connect to Wi-Fi, OTA Udpates and MQTT ---
  networkManager.init();
}

void loop() {
  // --- Handle OTA, MQTT updates in the loop ---
  networkManager.loop();
  
  // --- Publish a message every 5 seconds ---
  long now = millis();
  if (now - lastMsg > 10000) {
    lastMsg = now;    
    
    networkManager.debugPrint("Taking a picture!");
    camera_fb_t* fb = camera.captureFrame();
    if (!fb) {
      networkManager.debugPrint("Camera Capture Failed.");
      return;
    }
    if(networkManager.testUploadImage(serverUrl, fb->buf, fb->len)) {
      networkManager.debugPrint("Upload successful");
    }else{
      networkManager.debugPrint("Failed to upload image.");
    }
    // cleanup camera
    camera.cleanup(fb);
  }
}