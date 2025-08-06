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

    // tank steering
    int getCH1(); 
    int getCH2(); 

    // cam mount
    int getVRA();
    int getVRB();

private:
    IBusBM ibus;
    HardwareSerial &rcSerial;

    int rcCH1 = 0;
    int rcCH2 = 0;
    int rcVRA = 0;
    int rcVRB = 0;
};

#endif
