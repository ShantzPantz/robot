#include "config.h"
#include <Arduino.h>
#include <network_manager.h>
#include <driver/i2s.h>
#include <WebSocketsClient.h>

// I2S pin definitions
#define I2S_WS   25
#define I2S_SCK  26
#define I2S_SD   34  // Data input from mic

#define I2S_PORT          I2S_NUM_0
#define I2S_SAMPLE_RATE   16000
#define I2S_SAMPLE_BITS   16

#define CHUNK_SIZE        256   // samples per read (256 bytes)
#define FRAME_SIZE        1280  // total samples to send per frame (2560 bytes)

NetworkManager networkManager;
WebSocketsClient webSocket;

int16_t frameBuffer[FRAME_SIZE];
int16_t audioBuffer[CHUNK_SIZE];
size_t framePos = 0;          // current write position


// Queue monitoring
unsigned long lastPrint = 0;

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
    case WStype_BIN:
      Serial.printf("[WS] Binary len: %u\n", length);
      break;
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

  Serial.println("\n--- Starting ESP32 I2S Mic Test ---");

  // I2S configuration
  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = I2S_SAMPLE_RATE,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
      .communication_format = I2S_COMM_FORMAT_I2S,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 4,
      .dma_buf_len = 512,
      .use_apll = false,
      .tx_desc_auto_clear = false,
      .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
      .bck_io_num = I2S_SCK,
      .ws_io_num = I2S_WS,
      .data_out_num = I2S_PIN_NO_CHANGE,
      .data_in_num = I2S_SD
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);

  networkManager.init();
  delay(250);

  networkManager.debugPrint("Setting up I2S."); 

  // server address, port and URL
  webSocket.begin("shantz-ubuntu", 8000, "/ws"); // replace with your server IP
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
}

void sendFrame(int16_t* buffer, size_t numSamples) {
    if (webSocket.isConnected()) {
        // Send as binary data
        webSocket.sendBIN((uint8_t*)buffer, numSamples * sizeof(int16_t));       
        // Serial.println("Sending Frame");
    }
}

void loop() {
    networkManager.loop();
    webSocket.loop();

    size_t bytesRead = 0;
    esp_err_t result = i2s_read(I2S_PORT, audioBuffer, CHUNK_SIZE * sizeof(int16_t), &bytesRead, portMAX_DELAY);

    if (result == ESP_OK && bytesRead > 0) {
        size_t samplesRead = bytesRead / sizeof(int16_t);

        // Copy audio chunk into frame buffer
        memcpy(frameBuffer + framePos, audioBuffer, samplesRead * sizeof(int16_t));
        framePos += samplesRead;

        // If frame buffer is full, send it
        if (framePos >= FRAME_SIZE) {
            sendFrame(frameBuffer, FRAME_SIZE);  // send 2560 bytes
            framePos = 0; // reset for next frame
        }
    }
}
