#ifndef RS485SOIDMOISTURESENSOR_H
#define RS485SOIDMOISTURESENSOR_H

#include "../RS485.h"
//  #include "SensorLibAvr.h"
// #include "../SensorLib.h"

class RS485SoidMoistureSensor : public RS485 {

public:
  RS485Sensor SoidMoisture; 
  void beginSensor(uint32_t baud_rate) override {
    SoidMoisture.beginSensor(baud_rate);
  };
  // void setSerialPin(uint8_t rx_pin, uint8_t tx_pin) override {
  //   Temperature.setSerialPin( rx_pin,  tx_pin);
  //   // SoftwareSerial mod(26, 27);
  // };
  float getSensorValue() override {
    return (float)SoidMoisture.getSoidMoisture()/10;
  };

};
#endif