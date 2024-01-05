#ifndef RS485TEMPERATURESENSOR_H
#define RS485TEMPERATURESENSOR_H

#include "../RS485.h"
//  #include "SensorLibAvr.h"
// #include "../SensorLib.h"


class RS485TemperatureSensor : public RS485 {
// public:
//   RS485TemperatureSensor() : sensor() {}

//   void readData() override {
//     // Implementation for reading data from RS485 air sensor
//     Serial.println(sensor.getTemperature());
//     // Process the temperature value as needed...
//   }

//   void calibrate() override {
//     // Implementation for calibrating RS485 air sensor
//     Serial.println("Temperature");
//   }

// private:
//   RS485Sensor sensor;

public:
  RS485Sensor Temperature; 
  void beginSensor(uint32_t baud_rate) override {
    Temperature.beginSensor(baud_rate);
  };
  // void setSerialPin(uint8_t rx_pin, uint8_t tx_pin) override {
  //   Temperature.setSerialPin( rx_pin,  tx_pin);
  //   // SoftwareSerial mod(26, 27);
  // };
  float getSensorValue() override {
    float temp=0.0;
    for (int i=0; i<5 ; i++){
      temp+= Temperature.getTemperature()/10;
    }
    return temp/5;
  };

};
#endif