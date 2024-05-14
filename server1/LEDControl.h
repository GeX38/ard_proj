#ifndef LEDControl_h
#define LEDControl_h

#include "Arduino.h"

class LEDControl {
  public:
    LEDControl(int pinR, int pinG, int pinB);
    void setBrightness(int brightness);
    void setColor(int r, int g, int b);
    void fadeColor(int r, int g, int b);
  private:
    int _pinR;
    int _pinG;
    int _pinB;
    int _brightness;
};

#endif
