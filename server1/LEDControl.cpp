#include "Arduino.h"
#include "LEDControl.h"

LEDControl::LEDControl(int pinR, int pinG, int pinB) {
  pinMode(pinR, OUTPUT);
  pinMode(pinG, OUTPUT);
  pinMode(pinB, OUTPUT);
  _pinR = pinR;
  _pinG = pinG;
  _pinB = pinB;
}

void LEDControl::setBrightness(int brightness) {
  _brightness = brightness;
  analogWrite(_pinR, _brightness);
  analogWrite(_pinG, _brightness);
  analogWrite(_pinB, _brightness);
}

void LEDControl::setColor(int r, int g, int b) {
  analogWrite(_pinR, r);
  analogWrite(_pinG, g);
  analogWrite(_pinB, b);
}
void LEDControl::offLed(String currentRealTime, String ledOffTime){
  if (currentRealTime == ledOffTime){
    analogWrite(_pinR, 0);
    analogWrite(_pinG, 0);
    analogWrite(_pinB, 0);
  }
}

void LEDControl::onLed(String currentRealTime, String ledOnTime){
  if (currentRealTime == ledOffTime){
    analogWrite(_pinR, r);
    analogWrite(_pinG, g);
    analogWrite(_pinB, b);
  }
}

void LEDControl::fadeColor(int r, int g, int b) {
  for (int i = 0; i <= 255; i++) {
    analogWrite(_pinR, r * i / 255);
    analogWrite(_pinG, g * i / 255);
    analogWrite(_pinB, b * i / 255);
    delay(10);
  }
  for (int i = 255; i >= 0; i--) {
    analogWrite(_pinR, r * i / 255);
    analogWrite(_pinG, g * i / 255);
    analogWrite(_pinB, b * i / 255);
    delay(10);
  }
}
