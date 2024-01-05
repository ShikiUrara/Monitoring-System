
#ifndef ANALOGSALINITYSENSOR_H
#define ANALOGSALINITYSENSOR_H

#include "../Analog.h"
#include "SensorLib.h"


class AnalogSalinitySensor : public Analog {

public:
  AnalogSalinitySensor() : Sensor(nullptr) {}

  void beginSensor(uint8_t analog_pin, float voltage_reference, uint8_t resolution_bit) override {
    if (Sensor != nullptr) {
      delete Sensor;  // Delete the old object if it exists
    }
      Serial.println(analog_pin);
    Sensor = new AnalogSensor(analog_pin, voltage_reference, resolution_bit);  // Create a new object with the given parameters
  }

  float getSensorValue() override{
    return Sensor->getSalinity();
  };
  void calibrate(float slope, float intercept) override{
    Sensor->calibrateSalinity(slope, intercept);
  };

private:
  AnalogSensor* Sensor;


};

#endif 