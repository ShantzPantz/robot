#include "config.h"

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <network_manager.h>


NetworkManager::NetworkManager() 
  : mqttClient_(espClient_) {
    Serial.printf("NetworkManager Constructed.");
}

NetworkManager::~NetworkManager() {
    Serial.println("NetworkManager destroyed.");
}

void NetworkManager::init() {
    // --- Connect to Wi-Fi ---
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid_, password_);
    Serial.print("Connecting to WiFi");
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // --- Setup OTA Updates ---
    updateManager_.init(OTA_HOSTNAME);

    // --- Setup MQTT Client ---
    mqttClient_.setServer(mqtt_broker_, mqtt_port_);

      // --- Initial MQTT Connection (after Wi-Fi and OTA setup) ---
    reconnectMQTT();

    Serial.println("MQTT Setup Complete.");
    Serial.printf("Free Heap in setup(): %u bytes\n", ESP.getFreeHeap()); // Initial heap check
}

void NetworkManager::loop() {
    // --- Handle OTA updates in the loop ---
    updateManager_.checkForUpdate(); 

    // // --- Ensure MQTT client is connected ---
    // if (!mqttClient_.connected()) {
    //   reconnectMQTT();
    // }

    // // For MQTT to process messages 
    // mqttClient_.loop(); 
}

void NetworkManager::reconnectMQTT() {
  int max_retries = 5;
  int retries = 0;
  while (!mqttClient_.connected() && retries <= max_retries) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient_.connect(mqtt_client_id_, mqtt_username_, mqtt_password_)) {
      Serial.println("connected");
    } else {
      if (retries > max_retries) {
        Serial.println("Cannot connect to mqtt.");
      }else{
        Serial.print("failed, rc=");
        Serial.print(mqttClient_.state());
        Serial.println(" trying again in 5 seconds");
        retries ++;
        delay(5000);
      }      
    }
  }
}

void NetworkManager::debugPrint(const char* msg) {
#ifdef DEBUG
  Serial.println(msg);

  sendToMQTT(msg);
#endif
}

void NetworkManager::sendToMQTT(const char* msg) {
  if (mqttClient_.connected()) {
    mqttClient_.publish(mqtt_publish_topic_, msg);
  }
}

bool NetworkManager::testUploadImage(String uploadUrl, const uint8_t* buf, size_t len) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected");
    return false;
  }

  HTTPClient http;
  Serial.println("Trying to post to URL:");
  Serial.println(uploadUrl);
  http.begin(uploadUrl);  // Initialize HTTP connection
  http.addHeader("Content-Type", "image/jpeg");

  int httpResponseCode = http.POST(const_cast<uint8_t*>(buf), len);  // Send image as POST body

  if (httpResponseCode > 0) {
    Serial.printf("Upload succeeded. HTTP response: %d\n", httpResponseCode);
    http.end();
    return true;
  } else {
    Serial.printf("Upload failed. Error: %s\n", http.errorToString(httpResponseCode).c_str());
    http.end();
    return false;
  }
}