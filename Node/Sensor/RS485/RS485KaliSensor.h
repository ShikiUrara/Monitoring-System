#ifndef RS485KALISENSOR_H
#define RS485KALISENSOR_H

#include "../RS485.h"
//  #include "SensorLibAvr.h"
// #include "../SensorLib.h"

class RS485KaliSensor : public RS485 {

public:
  RS485Sensor Kali; 
  void beginSensor(uint32_t baud_rate) override {
    Kali.beginSensor(baud_rate);
  };
  // void setSerialPin(uint8_t rx_pin, uint8_t tx_pin) override {
  //   EC.setSerialPin( rx_pin,  tx_pin);
  //   // SoftwareSerial mod(26, 27);
  // };
  float getSensorValue() override {
    return (float)Kali.getKali();
  };

};
#endif