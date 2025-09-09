#include "config.h"
#include <Arduino.h>
#include <network_manager.h>
#include <driver/i2s.h>
#include <WebSocketsClient.h>
#include <AudioTools.h> 

NetworkManager networkManager;

// I2SStream object (full duplex, mic & dac)
I2SStream i2s; 
AudioInfo audioInfo(16000, 1, 16);  // 16kHz mono 16-bit PCM
bool isPlaying = false;
long lastBytesTime = 0;
long micCooldownMs = 2000; //2 secs

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("[WS] Disconnected");
      break;
    case WStype_CONNECTED:
      Serial.printf("[WS] Connected: %s\n", payload);
      break;
    case WStype_TEXT:      
      if (strcmp((char*)payload, "cmd:end_of_stream") == 0) {
        networkManager.debugPrint("Got cmd:end_of_stream messsage.");
        i2s.flush();
        networkManager.debugPrint("Sending cmd:end_of_playback message.");
        networkManager.getWebSocket().sendTXT("cmd:end_of_playback");
        isPlaying = false;
      }
      Serial.printf("[WS] Text: %s\n", payload);
      break;
    case WStype_BIN: {    
      isPlaying = true;        
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
    networkManager.init(webSocketEvent);
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
}

void loop() {
    networkManager.loop();   

    WebSocketsClient& webSocket = networkManager.getWebSocket();
    
    // uint8_t buffer[512];
    // int len = i2s.readBytes(buffer, sizeof(buffer));
    // if (len > 0 && webSocket.isConnected()) {
    //     webSocket.sendBIN(buffer, len);
    // }
    if (!isPlaying) {
      uint8_t buffer[512];
      int len = i2s.readBytes(buffer, sizeof(buffer));
      if (len > 0 && webSocket.isConnected()) {
          webSocket.sendBIN(buffer, len);
      }
  }
}