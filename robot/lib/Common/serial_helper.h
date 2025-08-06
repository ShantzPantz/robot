#ifndef SERIAL_HELPER_H
#define SERIAL_HELPER_H

#include <Arduino.h>

class SerialHelper {
public:
  SerialHelper(Stream& serial, const String& name = "") 
    : serialPort(serial), portName(name) {}

  void writeLine(const String& msg) {
    serialPort.println(msg);
  }

  bool available() {
    return serialPort.available() > 0;
  }

  String readLine() {
    return serialPort.readStringUntil('\n');
  }

  void debug(const String& prefix = "") {
    if (available()) {
      String line = readLine();
      if (portName != "") {
        Serial.print(portName);
        Serial.print(": ");
      }
      if (prefix != "") {
        Serial.print(prefix);
        Serial.print(": ");
      }
      Serial.println(line);
    }
  }

private:
  Stream& serialPort;
  String portName;
};

#endif
