#ifndef RS485DISTANCESENSOR_H
#define RS485DISTANCESENSOR_H

#include "../RS485.h"
//  #include "SensorLibAvr.h"
// #include "../SensorLib.h"

class RS485DistanceSensor : public RS485 {

public:
  RS485Sensor Distance; 
  void beginSensor(uint32_t baud_rate) override {
    Distance.beginSensor(baud_rate);
  };
  // void setSerialPin(uint8_t rx_pin, uint8_t tx_pin) override {
  //   Distance.setSerialPin( rx_pin,  tx_pin);
  //   // SoftwareSerial mod(26, 27);
  // };
  float getSensorValue() override {
    return (float)Distance.getDistance();
  };

};
#endif