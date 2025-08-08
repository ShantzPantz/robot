#include "config.h"

#include <Arduino.h>
#include <network_manager.h>
#include <vision.h>
#include "serial_helper.h"

long lastMsg = 0;
int value = 0;
const char* serverUrl = "http://192.168.2.64:5000/test_image_upload";

Vision camera;
NetworkManager networkManager;
SerialHelper mega(Serial, "mega");

void setup() {
  Serial.begin(9600);
  // --- Initialize Camera --- 
  camera.initialize();

  // --- Connect to Wi-Fi, OTA Udpates and MQTT ---
  networkManager.init();
}

void loop() {
  // --- Handle OTA, MQTT updates in the loop ---
  networkManager.loop();

  if (mega.available()) {      
    networkManager.debugPrint(mega.readLine().c_str());
  }
  
  // --- Publish a message every 5 seconds ---
  long now = millis();
  if (now - lastMsg > 10000) {
    lastMsg = now;    

    networkManager.debugPrint("Taking a picture! Again.");
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