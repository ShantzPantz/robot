#include "config.h"

#include <Arduino.h>
#include <network_manager.h>

long lastMsg = 0;
int value = 0;

NetworkManager networkManager;

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n--- Starting esp32cam_audio ---");

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
    
    networkManager.debugPrint("It is working!");  
  }
}