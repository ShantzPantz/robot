
#include <Arduino.h>
#include "rc_input.h"

RCInput::RCInput(HardwareSerial &serial) : rcSerial(serial) {}

void RCInput::setup() {
    rcSerial.begin(115200);
    ibus.begin(rcSerial);
}

void RCInput::loop() {
    rcCH1 = readChannel(0, -100, 100, 0); 
    rcCH2 = readChannel(1, -100, 100, 0); 
    Serial.print("CH1: ");
    Serial.print(rcCH1);
    Serial.print(" | CH2: ");
    Serial.println(rcCH2);
}

int RCInput::readChannel(byte channelInput, int minLimit, int maxLimit, int defaultValue) {
    uint16_t ch = ibus.readChannel(channelInput);
    if (ch < 100) return defaultValue;
    return map(ch, 1000, 2000, minLimit, maxLimit);
}

bool RCInput::readSwitch(byte channelInput, bool defaultValue) {
    int intDefaultValue = defaultValue ? 100 : 0;
    int ch = readChannel(channelInput, 0, 100, intDefaultValue);
    return (ch > 50);
}

int RCInput::getCH1() {
    return rcCH1;
}

int RCInput::getCH2() {
    return rcCH2;
}
