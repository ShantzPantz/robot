#include "ota_update_manager.h"
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>

OtaUpdateManager::OtaUpdateManager() {
        Serial.printf("OtaUpdateManager Constructed.");
    }

OtaUpdateManager::~OtaUpdateManager() {
    Serial.println("OtaUpdateManager destroyed.");
}

void OtaUpdateManager::init(const char* ota_hostname) {
    ota_hostname_ = ota_hostname;
    Serial.println("Initializing OTA Updater with hostname:");
    Serial.println(ota_hostname);

    // Start mDNS service
    if (!MDNS.begin(ota_hostname)) {
        Serial.println("Error starting mDNS");
    } else {
        Serial.println("mDNS responder started");
        MDNS.addService("ota", "tcp", 3232); 
    }

    ArduinoOTA.setHostname(ota_hostname);
    // If adding a password, pass it in here. 
    // ArduinoOTA.setPassword(ota_password_);

    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
        else // U_SPIFFS
        type = "filesystem";
        Serial.println("Start updating " + type);
        // Add a memory check right at the start of OTA
        Serial.printf("Free Heap BEFORE OTA: %u bytes\n", ESP.getFreeHeap());
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
        Serial.printf("Free Heap AFTER OTA Error: %u bytes\n", ESP.getFreeHeap()); // Check heap on error
    });

    ArduinoOTA.begin();
    Serial.println("OTA Initialized.");
}

void OtaUpdateManager::checkForUpdate() {
    ArduinoOTA.handle();
}