#ifndef PIDCONTROLLER_H
#define PIDCONTROLLER_H

#include <PID_v1_bc.h>
#include <microDS18B20.h>
template<int pin, int PIN_HEATER>
class PIDController {
  private:
    MicroDS18B20<pin> ts;
    double Setpoint, Input, Output;
    double Kp=6, Ki=1, Kd=0.01;
    PID myPID;

  public:
    PIDController() : myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT) {
      myPID.SetMode(AUTOMATIC);
      myPID.SetOutputLimits(0, 255);
    }

    void heating(double desiredTemp) {
      static uint32_t timer = millis();
      if(millis() - timer >= 1000){
        timer = millis();
        if(ts.readTemp()){
          Input = ts.getTemp();
        }
        ts.requestTemp();
      }
      Setpoint = desiredTemp;
      myPID.Compute();
      analogWrite(PIN_HEATER, Output);
    }
    String readTemperature() {
      if (ts.readTemp()) {
        ts.requestTemp();
        return String(ts.getTemp());
        }
      return "Sensor data not available";
    }
};

#endif
