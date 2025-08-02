#ifndef RCINPUT_H
#define RCINPUT_H

#include <IBusBM.h>

class RCInput {
public:
    RCInput(HardwareSerial &serial);

    void setup();
    void loop();

    int readChannel(byte channelInput, int minLimit, int maxLimit, int defaultValue);
    bool readSwitch(byte channelInput, bool defaultValue);

    int getCH1(); // Left - Right
    int getCH2(); // Forward - Reverse

private:
    IBusBM ibus;
    HardwareSerial &rcSerial;

    int rcCH1 = 0;
    int rcCH2 = 0;
};

#endif
