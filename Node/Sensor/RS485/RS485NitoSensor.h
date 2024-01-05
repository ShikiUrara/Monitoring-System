#ifndef RS485NITOSENSOR_H
#define RS485NITOSENSOR_H

#include "../RS485.h"
//  #include "SensorLibAvr.h"
// #include "../SensorLib.h"

class RS485NitoSensor : public RS485 {

public:
  RS485Sensor Nito; 
  void beginSensor(uint32_t baud_rate) override {
    Nito.beginSensor(baud_rate);
  };
  // void setSerialPin(uint8_t rx_pin, uint8_t tx_pin) override {
  //   Nito.setSerialPin( rx_pin,  tx_pin);
  //   // SoftwareSerial mod(26, 27);
  // };
  float getSensorValue() override {
    return (float)Nito.getNito();
  };

};
#endif