#ifndef RS485PHSENSOR_H
#define RS485PHSENSOR_H

#include "../RS485.h"
//  #include "SensorLibAvr.h"
// #include "../SensorLib.h"

class RS485PHSensor : public RS485 {

public:
  RS485Sensor PH; 
  void beginSensor(uint32_t baud_rate) override {
    PH.beginSensor(baud_rate);
  };
  // void setSerialPin(uint8_t rx_pin, uint8_t tx_pin) override {
  //   PH.setSerialPin( rx_pin,  tx_pin);
  //   // SoftwareSerial mod(26, 27);
  // };
  float getSensorValue() override {
    return (float)PH.getPH()/100;
  };

};
#endif