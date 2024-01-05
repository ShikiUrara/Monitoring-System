#ifndef ANALOGTEMPERATURESENSOR_H
#define ANALOGTEMPERATURESENSOR_H

#include "../Analog.h"
// #include "../SensorLib.h"
#include "SensorLib.h"
// #include "SensorLibAvr.h"

class AnalogTemperatureSensor : public Analog {

public:
  AnalogTemperatureSensor() : Sensor(nullptr) {}

  void beginSensor(uint8_t analog_pin, float voltage_reference, uint8_t resolution_bit) override {
    if (Sensor != nullptr) {
      delete Sensor;  // Delete the old object if it exists
    }
    Sensor = new AnalogSensor(analog_pin, voltage_reference, resolution_bit);  // Create a new object with the given parameters
  }

  float getSensorValue() override{
    float temp=0.0;
    for (int i=0; i<5 ; i++){
      temp+= Sensor->getTemperature();
    }
    return temp/5;
  };
  void calibrate(float slope, float intercept) override{
    Sensor->calibrateTemperature(slope, intercept);
  };

private:
  AnalogSensor* Sensor;


};

#endif 