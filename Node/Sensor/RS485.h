#ifndef RS485SENSOR_H
#define RS485SENSOR_H

#include "SensorLib.h"

class RS485  {
public:
  //RS485 Method
  virtual void beginSensor(uint32_t baud_rate);
  // virtual void setSerialPin(uint8_t rx_pin, uint8_t tx_pin);
  virtual float getSensorValue();
  //Dung de tinh toan Sensor khong co trong thu vien
  // virtual int getSensorValue(byte _sendFrame[],  uint8_t size_response_frame);
  // virtual int getSensorValue(byte _sendFrame[],  uint8_t size_response_frame, uint8_t position);
};

#endif


