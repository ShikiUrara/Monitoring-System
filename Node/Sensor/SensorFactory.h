#ifndef SENSORFACTORY_H
#define SENSORFACTORY_H


#include "Analog/AnalogNH4Sensor.h"
#include "Analog/AnalogTemperatureSensor.h"
#include "Analog/AnalogSalinitySensor.h"


#include "RS485/RS485TemperatureSensor.h"
#include "RS485/RS485SoidMoistureSensor.h"
#include "RS485/RS485DistanceSensor.h"
#include "RS485/RS485ECSensor.h"
#include "RS485/RS485PHSensor.h"
#include "RS485/RS485PhotphoSensor.h"
#include "RS485/RS485NitoSensor.h"
#include "RS485/RS485KaliSensor.h"

// Include other sensor classes...

class SensorFactory {
public:
  Analog* createSensorAnalog(String type, String protocol) {
   
    if (type == "NH4" && protocol == "Analog") {
      return new AnalogNH4Sensor();
    }
    else if (type == "Temperature" && protocol == "Analog") {
      return new AnalogTemperatureSensor();
    }
    else if (type == "Salinity" && protocol == "Analog") {
      return new AnalogSalinitySensor();
    }
    
    // Add other conditions for other sensor types and protocols...
    else {
      return nullptr;
    }
  }

  RS485* createSensorRS485(String type, String protocol) {

    if (type == "Temperature" && protocol == "RS485") {
      return new RS485TemperatureSensor();
    }
    else if (type == "SoidMoisture" && protocol == "RS485") {
      return new RS485SoidMoistureSensor();
    }
    else if (type == "Distance" && protocol == "RS485") {
      return new RS485DistanceSensor();
    }
    else if (type == "EC" && protocol == "RS485") {
      return new RS485DistanceSensor();
    }
    else if (type == "PH" && protocol == "RS485") {
      return new RS485DistanceSensor();
    }
    else if (type == "Nito" && protocol == "RS485") {
      return new RS485DistanceSensor();
    }
    else if (type == "Photpho" && protocol == "RS485") {
      return new RS485DistanceSensor();
    }
    else if (type == "Kali" && protocol == "RS485") {
      return new RS485DistanceSensor();
    }
    
    else {
      return nullptr;
    }
  }
};

#endif


