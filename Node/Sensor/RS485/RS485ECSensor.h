#ifndef RS485ECSENSOR_H
#define RS485ECSENSOR_H

#include "../RS485.h"
//  #include "SensorLibAvr.h"
// #include "../SensorLib.h"

class RS485ECSensor : public RS485 {

public:
  RS485Sensor EC; 
  void beginSensor(uint32_t baud_rate) override {
    EC.beginSensor(baud_rate);
  };
  // void setSerialPin(uint8_t rx_pin, uint8_t tx_pin) override {
  //   EC.setSerialPin( rx_pin,  tx_pin);
  //   // SoftwareSerial mod(26, 27);
  // };
  float getSensorValue() override {
    return (float)EC.getEC();
  };

};
#endif