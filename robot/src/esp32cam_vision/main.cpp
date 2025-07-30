#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>   // New: For Over-The-Air updates
#include <ESPmDNS.h>      // New: For mDNS (network discovery)
#include <ota_update_manager.h>
#include <config.h>

// --- Wi-Fi Credentials ---
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// --- MQTT Broker Details ---
// IMPORTANT: Use the private IP address of your Windows PC where Mosquitto is running
const char* mqtt_broker = MQTT_BROKER; 
const int mqtt_port = MQTT_PORT;
const char* mqtt_client_id = MQTT_CLIENT_ID; 
const char* mqtt_username = MQTT_USERNAME;
const char* mqtt_password = MQTT_PASSWORD;
const char* mqtt_publish_topic = MQTT_TOPIC;

// --- OTA Settings ---
// Must match upload_port in platformio.ini (without .local)
const char* ota_hostname = OTA_HOSTNAME; // assigned alias in router
// Must match upload_flags --auth in platformio.ini
// const char* ota_password = OTA_PASSWORD; // if I end up enabling passwords


WiFiClient espClient;
PubSubClient mqttClient(espClient);
OtaUpdateManager updateManager;

long lastMsg = 0;
int value = 0;

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect(mqtt_client_id, mqtt_username, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" trying again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n--- Starting MQTT Bare Minimum Sketch with OTA ---");

  // --- Connect to Wi-Fi ---
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // --- Setup MQTT Client ---
  mqttClient.setServer(mqtt_broker, mqtt_port);

  // --- Setup OTA Updates ---
  updateManager.init(ota_hostname);

  // --- Initial MQTT Connection (after Wi-Fi and OTA setup) ---
  reconnectMQTT();

  Serial.println("MQTT Setup Complete.");
  Serial.printf("Free Heap in setup(): %u bytes\n", ESP.getFreeHeap()); // Initial heap check
}

void loop() {
  // --- Handle OTA updates in the loop ---
  updateManager.checkForUpdate(); 

  // --- Ensure MQTT client is connected ---
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop(); // For MQTT to process messages

  // --- Publish a message every 5 seconds ---
  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    value++;
    char msg[75];
    snprintf(msg, sizeof(msg), "We're doing it, and we're doing it well. Message Count: %d, Free Heap: %u", value, ESP.getFreeHeap());

    Serial.print("Publishing MQTT message: ");
    Serial.println(msg);

    mqttClient.publish(mqtt_publish_topic, msg);
  }
}