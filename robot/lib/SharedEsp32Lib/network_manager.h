// Helper class for handling wifi and mqtt messaging
#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include "config.h"

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WebSocketsClient.h>
#include <ota_update_manager.h>

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();

    void init(std::function<void(WStype_t, uint8_t*, size_t)> wsEventHandler);
    void reconnectMQTT();
    void loop();

    void debugPrint(const char* msg);
    void sendToMQTT(const char* msg);

    bool uploadImage(String url, const uint8_t* buf, size_t len);
    
    bool isWifiConnected();

    WebSocketsClient& getWebSocket();

private:
    WiFiClient espClient_;
    PubSubClient mqttClient_;
    OtaUpdateManager updateManager_;
    WebSocketsClient webSocket_;

    // --- Wi-Fi Credentials ---
    const char* ssid_ = WIFI_SSID;
    const char* password_ = WIFI_PASSWORD;

    // --- MQTT Broker Details ---
    const char* mqtt_broker_ = MQTT_BROKER; 
    const int mqtt_port_ = MQTT_PORT;
    const char* mqtt_client_id_ = MQTT_CLIENT_ID; 
    const char* mqtt_username_ = MQTT_USERNAME;
    const char* mqtt_password_ = MQTT_PASSWORD;
    const char* mqtt_publish_topic_ = MQTT_TOPIC;
};
#endif