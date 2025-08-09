#include "config.h"

#include <Arduino.h>
#include <network_manager.h>
#include <vision.h>
#include "serial_helper.h"

long lastMsg = 0;
int value = 0;
const char* serverUrl = "http://192.168.2.64:5000/image_upload";

Vision camera;
NetworkManager networkManager;
SerialHelper mega(Serial, "mega");

enum State {
  ACTIVE,
  PASSIVE
};

State currentState = State::PASSIVE;
String currentID = "0";

void setup() {
  Serial.begin(9600);
  // --- Initialize Camera --- 
  camera.initialize();

  // --- Connect to Wi-Fi, OTA Udpates and MQTT ---
  networkManager.init();
}

void processCommand(String msg) {
  String cmd;
  String cmd_id;

  // Find the position of the colon
  int colonIndex = msg.indexOf(':');

  // Check if the colon was found
  if (colonIndex != -1) {
    // Extract the command part (before the colon)
    cmd = msg.substring(0, colonIndex);
    // Extract the ID (after the colon)
    cmd_id = msg.substring(colonIndex + 1);
    
    if(cmd == "START") {
      currentState = State::ACTIVE;
      currentID = cmd_id;
    } else if (cmd == "END") {
      currentState = State::PASSIVE;
    }    
  }
}

void loop() {
  // --- Handle OTA, MQTT updates in the loop ---
  networkManager.loop();

  if (mega.available()) {      
    String msg = mega.readLine();
    networkManager.debugPrint(msg.c_str());
    processCommand(msg);
  }
  
  if (currentState == State::ACTIVE) {
    // --- Publish a message every 5 seconds while active---
    long now = millis();
    if (now - lastMsg > 5000) {
      lastMsg = now;    

      networkManager.debugPrint("Taking a picture! Again.");
      camera_fb_t* fb = camera.captureFrame();
      if (!fb) {
        networkManager.debugPrint("Camera Capture Failed.");
        return;
      }
      
      if(networkManager.uploadImage(serverUrl + String("?rid=") + currentID, fb->buf, fb->len)) {
        networkManager.debugPrint("Upload successful");
      }else{
        networkManager.debugPrint("Failed to upload image.");
      }
      // cleanup camera
      camera.cleanup(fb);
    }
  }  
}