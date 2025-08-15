#ifndef RCINPUT_H
#define RCINPUT_H

#include "config.h"
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

    // camera 
    int getCH3();
    int getCH4();    

    // switches A & B
    bool getSWA();
    bool getSWB();

private:
    IBusBM ibus;
    HardwareSerial &rcSerial;

    int rcCH1 = 0;
    int rcCH2 = 0;
    int rcCH3 = 0;
    int rcCH4 = 0;
    bool rcSWA = false;
    bool rcSWB = false;
};

#endif
