#ifndef RS485PHOTPHOSENSOR_H
#define RS485PHOTPHOSENSOR_H

#include "../RS485.h"
//  #include "SensorLibAvr.h"
// #include "../SensorLib.h"

class RS485PhotphoSensor : public RS485 {

public:
  RS485Sensor Photpho; 
  void beginSensor(uint32_t baud_rate) override {
    Photpho.beginSensor(baud_rate);
  };
  // void setSerialPin(uint8_t rx_pin, uint8_t tx_pin) override {
  //   Photpho.setSerialPin( rx_pin,  tx_pin);
  //   // SoftwareSerial mod(26, 27);
  // };
  float getSensorValue() override {
    return (float)Photpho.getPhotpho();
  };

};
#endif