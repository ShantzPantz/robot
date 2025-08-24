#include "config.h"
#include <Arduino.h>
#include <network_manager.h>
#include <driver/i2s.h>
#include <WebSocketsClient.h>
#include "AudioTools.h"

WebSocketsClient webSocket;
NetworkManager networkManager;


// I2SStream object (full duplex, mic & dac)
I2SStream i2s; 
AudioInfo audioInfo(16000, 1, 16);  // 16kHz mono 16-bit PCM

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("[WS] Disconnected");
      break;
    case WStype_CONNECTED:
      Serial.printf("[WS] Connected: %s\n", payload);
      break;
    case WStype_TEXT:
      Serial.printf("[WS] Text: %s\n", payload);
      break;
    case WStype_BIN: {
      //   Serial.printf("[WS] Binary len: %u\n", length);	 
      i2s.write(payload, length);
      break;
    }
    case WStype_ERROR:
      Serial.println("[WS] Error occurred!");
      break;
    case WStype_PING:
      Serial.println("[WS] Ping!");
      break;
    case WStype_PONG:
      Serial.println("[WS] Pong!");
      break;
  }
}

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n--- Starting esp32_audio ---");

    // --- Connect to Wi-Fi and handle network services ---
    networkManager.init();
    delay(250);

    // Setup I2S microphone
    auto cfg = i2s.defaultConfig(RXTX_MODE);
    cfg.sample_rate = audioInfo.sample_rate;
    cfg.bits_per_sample = audioInfo.bits_per_sample;
    cfg.channels = audioInfo.channels;
    cfg.i2s_format = I2S_PHILIPS_FORMAT; 
    cfg.pin_bck = 26;
    cfg.pin_ws = 25;
    cfg.pin_data_rx = 34; // input pin
    cfg.pin_data = 22; // output pin
    i2s.begin(cfg);

    // Setup websockets
    webSocket.begin("shantz-ubuntu", 8000, "/audio_test");
    webSocket.onEvent(webSocketEvent);
    // webSocket.setAuthorization("user", "Password"); // add when ready	
    webSocket.setReconnectInterval(5000);	
	  webSocket.enableHeartbeat(5000, 15000, 30000);
}

void loop() {
    networkManager.loop();
    webSocket.loop();

    // Read from mic
    uint8_t buffer[512];
    int len = i2s.readBytes(buffer, sizeof(buffer));
    if (len > 0 && webSocket.isConnected()) {
        webSocket.sendBIN(buffer, len);
    }
}