#ifndef ANALOGNH4SENSOR_H
#define ANALOGNH4SENSOR_H

#include "../Analog.h"
// #include "../SensorLib.h"
#include "SensorLib.h"
// #include "SensorLibAvr.h"

class AnalogNH4Sensor : public Analog {

public:
  AnalogNH4Sensor() : Sensor(nullptr) {}

  void beginSensor(uint8_t analog_pin, float voltage_reference, uint8_t resolution_bit) override {
    if (Sensor != nullptr) {
      delete Sensor;  // Delete the old object if it exists
    }
    Sensor = new AnalogSensor(analog_pin, voltage_reference, resolution_bit);  // Create a new object with the given parameters
  }

  float getSensorValue() override{
    return Sensor->getNH4();
  };
  void calibrate(float slope, float intercept) override{
    Sensor->calibrateNH4(slope, intercept);
  };

private:
  AnalogSensor* Sensor;


};

#endif 