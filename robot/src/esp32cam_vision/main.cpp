#include "config.h"

#include <Arduino.h>
#include <network_manager.h>
#include <vision.h>
#include "serial_helper.h"

long lastMsg = 0;
int value = 0;

Vision camera;
NetworkManager networkManager;
SerialHelper mega(Serial, "mega");

enum State {
  ACTIVE,
  PASSIVE
};

State currentState = State::PASSIVE;
String currentID = "0";

void process_command(char* cmd) {
  if (strcmp(cmd, "request_image") == 0) {
    camera_fb_t* fb = camera.captureFrame();
    if (!fb) {
      networkManager.debugPrint("Camera Capture Failed.");
      return;
    }

    // send back to websocket
    WebSocketsClient& webSocket = networkManager.getWebSocket();
    if(webSocket.isConnected()) {
      webSocket.sendBIN(fb->buf, fb->len);
    }
          
    // cleanup camera
    camera.cleanup(fb);
  }
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      networkManager.debugPrint("[WS] Disconnected");
      break;
    case WStype_CONNECTED:
      networkManager.debugPrint("[WS] Connected");
      break;
    case WStype_TEXT:
      networkManager.debugPrint("[WS] Got Text...");
      process_command((char*)payload);
      break;
    case WStype_BIN: {
      networkManager.debugPrint("[WS] Got Binary...");    
      break;
    }
    case WStype_ERROR:
      networkManager.debugPrint("[WS] Error occurred!");
      break;
    case WStype_PING:
      networkManager.debugPrint("[WS] Ping!");
      break;
    case WStype_PONG:
      networkManager.debugPrint("[WS] Pong!");
      break;
  }
}

void setup() {
  Serial.begin(9600);
  // --- Initialize Camera --- 
  camera.initialize();

  // --- Connect to Wi-Fi, OTA Udpates and MQTT ---
  networkManager.init(webSocketEvent);
  networkManager.debugPrint("Setup Complete.");
}

// void processCommand(String msg) {
//   String cmd;
//   String cmd_id;

//   // Find the position of the colon
//   int colonIndex = msg.indexOf(':');

//   // Check if the colon was found
//   if (colonIndex != -1) {
//     // Extract the command part (before the colon)
//     cmd = msg.substring(0, colonIndex);
//     // Extract the ID (after the colon)
//     cmd_id = msg.substring(colonIndex + 1);
    
//     if(cmd == "START") {
//       currentState = State::ACTIVE;
//       currentID = cmd_id;
//     } else if (cmd == "END") {
//       currentState = State::PASSIVE;
//     }    
//   }
// }

void loop() {
  // --- Handle OTA, MQTT updates in the loop ---
  networkManager.loop();

  // if (mega.available()) {      
  //   String msg = mega.readLine();
  //   networkManager.debugPrint(msg.c_str());
  //   processCommand(msg);
  // }
    
}