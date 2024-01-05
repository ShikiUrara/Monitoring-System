#ifndef ANALOGSENSOR_H
#define ANALOGSENSOR_H



class Analog {
public:

  virtual void beginSensor(uint8_t analog_pin, float voltage_reference, uint8_t resolution_bit);
  virtual float getSensorValue();
  virtual void calibrate(float slope, float intercept);
  
  //Dung de tinh toan Sensor khong co trong thu vien
  // int getRawValue(){
  //   return SensorNew->getRawValue();
  // };
  // float getVoltage(){
  //   return SensorNew->getVoltage();
  // };

};

#endif
