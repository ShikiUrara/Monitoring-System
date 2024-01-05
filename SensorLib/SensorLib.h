//xtensa

#ifndef SensorLib_h
#define SensorLib_h

#include "Arduino.h"

class RS485Sensor {
public:
  RS485Sensor();
  void beginSensor(uint32_t baud_rate);
  int getSensorValue(byte _sendFrame[],  uint8_t size_response_frame);
  int getSensorValue(byte _sendFrame[],  uint8_t size_response_frame, uint8_t position);
  int getDistance();
  int getTemperature();
  int getHumidity();
  int getPH();
  int getNito();
  int getPhotpho();
  int getKali();
  int getSoidMoisture();
  int getEC();

private:
  uint32_t _baud_rate;
  byte _queryTemHum[8] = { 0x01, 0x03, 0x00, 0x12, 0x00, 0x02, 0x64, 0x0E };
  byte _queryN[8] = { 0x01, 0x03, 0x00, 0x1E, 0x00, 0x01, 0xE4, 0x0C };
  byte _queryP[8] = { 0x01, 0x03, 0x00, 0x1F, 0x00, 0x01, 0xB5, 0xCC };
  byte _queryK[8] = { 0x01, 0x03, 0x00, 0x20, 0x00, 0x01, 0x85, 0xC0 };
  byte _querySM[8] = { 0x01, 0x03, 0x00, 0x12, 0x00, 0x01, 0x24, 0x0F };
  byte _queryEC[8] = { 0x01, 0x03, 0x00, 0x15, 0x00, 0x01, 0x95, 0xCE };
  byte _queryPH[8] = { 0x01, 0x03, 0x00, 0x06, 0x00, 0x01, 0x64, 0x0B };
  byte _queryDis[8] = { 0x50, 0x03, 0x00, 0x34, 0x00, 0x01, 0xC8, 0x45 };
  
  String decToHex(int decValue);
  int hexToDec(String hexString);
};

class AnalogSensor {
public:
AnalogSensor(uint8_t analog_pin, float voltage_reference, uint8_t resolution_bit);
int getRawValue();
float getVoltage();
void calibrateSalinity(float slope_salinity, float intercept_salinity);
float getSalinity() ;
float getPH();
void calibratePH(float slope_PH, float intercept_PH);
float getNH4();
void calibrateNH4(float slope_NH4, float intercept_NH4);
float getTemperature();
void calibrateTemperature(float slope_temp, float intercept_temp);

private:
uint8_t _analog_pin;
float _voltage_reference;
uint8_t _resolution_bit;
uint16_t _ADC_value;
float _slope_salinity = 16.3;
float _intercept_salinity = 0;
float _slope_PH;
float _intercept_PH = 0;
float _slope_NH4;
float _intercept_NH4;
float _slope_temp=1;
float _intercept_temp=0;
};

#endif