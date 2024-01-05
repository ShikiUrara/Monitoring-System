#include "Arduino.h"
#include "SensorLib.h"

RS485Sensor ::RS485Sensor()
{
}

void RS485Sensor::beginSensor(uint32_t baud_rate)
{
  _baud_rate = baud_rate;
}

String RS485Sensor ::decToHex(int decValue)
{
  char hexString[6];
  sprintf(hexString, "%04X", decValue);
  int length = strlen(hexString);
  for (int i = 0; i < length; i++)
  {
    if (hexString[i] != '0')
    {
      return String(hexString + i);
    }
  }
  return "0";
}

int RS485Sensor ::hexToDec(String hexString)
{
  const char *hexChar = hexString.c_str();
  int decValue = strtol(hexChar, NULL, 16);
  return decValue;
}

int RS485Sensor ::getSensorValue(byte send_frame[], uint8_t size_response_frame)
{
  return getSensorValue(send_frame, size_response_frame, 0);
}

int RS485Sensor ::getSensorValue(byte send_frame[], uint8_t size_response_frame, uint8_t position)
{
  Serial2.begin(_baud_rate);
  byte arr_receive[size_response_frame] = {0};
  int value = 0;
  Serial2.write(send_frame, 8);
  delay(100);
  if (Serial2.available() >= size_response_frame)
  {
    Serial2.readBytes(arr_receive, size_response_frame);
  }
  if (arr_receive[size_response_frame - 1] != 0)
  {
    String part_1;
    String part_2;
    String part_12;
    switch (size_response_frame)
    {
    case 7:
      part_1 = (String)decToHex(arr_receive[3]);
      part_2 = (String)decToHex(arr_receive[4]);
      part_12 = part_1 + part_2;
      value = hexToDec(part_12);
      break;
    case 9:
      switch (position)
      {
      case 1:
        part_1 = (String)decToHex(arr_receive[3]);
        part_2 = (String)decToHex(arr_receive[4]);
        part_12 = part_1 + part_2;
        value = hexToDec(part_12);
        break;
      case 2:
        part_1 = (String)decToHex(arr_receive[5]);
        part_2 = (String)decToHex(arr_receive[6]);
        part_12 = part_1 + part_2;
        value = hexToDec(part_12);
        break;
      }
      break;
    }
  }
  else
  {
  }
  return value;
}

int RS485Sensor ::getDistance()
{
  return getSensorValue(_queryDis, 7);
}

int RS485Sensor ::getTemperature()
{
  return getSensorValue(_queryTemHum, 9, 2);
}

int RS485Sensor ::getHumidity()
{
  return getSensorValue(_queryTemHum, 9, 1);
}

int RS485Sensor ::getPH()
{
  return getSensorValue(_queryPH, 7);
}

int RS485Sensor ::getNito()
{
  return getSensorValue(_queryN, 7);
}

int RS485Sensor ::getPhotpho()
{
  return getSensorValue(_queryP, 7);
}

int RS485Sensor ::getKali()
{
  return getSensorValue(_queryK, 7);
}

int RS485Sensor ::getEC()
{
  return getSensorValue(_queryEC, 7);
}

int RS485Sensor ::getSoidMoisture()
{
  return getSensorValue(_querySM, 7);
}

AnalogSensor ::AnalogSensor(uint8_t analog_pin, float voltage_reference, uint8_t resolution_bit)
{
  _analog_pin = analog_pin;
  _voltage_reference = voltage_reference;
  _resolution_bit = resolution_bit;
  if (_resolution_bit == 10)
  {
    _ADC_value = 1023;
  }
  if (_resolution_bit == 12)
  {
    _ADC_value = 4095;
  }
  pinMode(_analog_pin, INPUT);
}

int AnalogSensor ::getRawValue()
{
  int raw_value = analogRead(_analog_pin);
  return raw_value;
}

float AnalogSensor ::getVoltage()
{
  int raw_value = getRawValue();
  // Serial.print("raw: ");
  // Serial.println(raw_value);
  float voltage = (float(raw_value) / float(_ADC_value))*_voltage_reference;
  // Serial.print("vol: ");
  // Serial.println(voltage);
  voltage= voltage*2;
  return voltage;
}

void AnalogSensor ::calibrateSalinity(float slope_salinity, float intercept_salinity)
{
  _slope_salinity = slope_salinity;
  _intercept_salinity = intercept_salinity;
}

float AnalogSensor ::getSalinity()
{
  float voltage = getVoltage();
  float Sal = voltage * _slope_salinity + _intercept_salinity;
  return Sal;
}

float AnalogSensor ::getPH()
{
  float voltage = getVoltage();
  float PH = voltage * _slope_PH + _intercept_PH;
  return PH;
}

void AnalogSensor ::calibratePH(float slope_PH, float intercept_PH)
{
  _slope_PH = slope_PH;
  _intercept_PH = _intercept_PH;
}

float AnalogSensor ::getNH4()
{
  float voltage = getVoltage();
  float NH4 = voltage * _slope_NH4 + _intercept_NH4;
  return NH4;
}

void AnalogSensor ::calibrateNH4(float slope_NH4, float intercept_NH4)
{
  _slope_NH4 = slope_NH4;
  _intercept_NH4 = intercept_NH4;
}

// float AnalogSensor ::getTemperature()
// {
//   const float Ko = 1.02119e-3;
//   const float K1 = 2.22468e-4;
//   const float K2 = 1.33342e-7;
//   uint16_t measured_resistance = 20000;
//   float temp = 1.0 / (Ko + K1 * logf(1000.0 * measured_resistance) + K2 * powf(logf(1000.0 * measured_resistance), 3)) - 273.15;
//   temp = temp* _slope_temp + _intercept_temp;
//   return temp;
// }

float AnalogSensor ::getTemperature()
{
  float voltage = getVoltage();
  const float K0 = 1.02119e-3;
  const float K1 = 2.22468e-4;
  const float K2 = 1.33342e-7;
  uint16_t measured_resistance = 20000;
  long resistance = (measured_resistance * voltage ) / (5.0 - voltage);
  float _logR = log(resistance); // Saving the Log(resistance) so not to calculate  it 4 times later
  float temp = 1 / (K0 + (K1 * _logR + (K2 * _logR * _logR * _logR)));
  temp = temp - 273.15;
  // float temp = 1.0 / (Ko + K1 * logf(resistance) + K2 * powf(logf(1000.0 * resistance), 3)) - 273.15;
  temp = temp* _slope_temp + _intercept_temp;
  return temp;
}

void AnalogSensor ::calibrateTemperature(float slope_temp, float intercept_temp)
{
  _slope_temp = slope_temp;
  _intercept_temp = intercept_temp;
}