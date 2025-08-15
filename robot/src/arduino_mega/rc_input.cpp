
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
    rcCH3 = readChannel(2, -100, 100, 0);
    rcCH4 = readChannel(3, -100, 100, 0);    
    rcSWA = readSwitch(4, false);
    rcSWB = readSwitch(5, false);

// #ifdef DEBUG
//     static long last_print = 0;
//     long now = millis();
//     if(now - last_print > 5000) {
//         for(int i=0; i<=5; i++) {
//             Serial.print("Ch ");
//             Serial.print(i);
//             Serial.print("=");
//             Serial.print(ibus.readChannel(i));
//             Serial.print("  |  ");
//         }
//         Serial.println("");
//         Serial.println("-------------------------------------------------");
//         last_print = now;
//     }
// #endif
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

int RCInput::getCH3() {
    return rcCH3;
}

int RCInput::getCH4() {
    return rcCH4;
}

bool RCInput::getSWA() {
    return rcSWA;
}

bool RCInput::getSWB() {
    return rcSWB;
}
