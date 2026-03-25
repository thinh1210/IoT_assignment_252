#ifndef DHT_SENSOR_H
#define DHT_SENSOR_H

#include <Arduino.h>
#include "config.h"

class DHTSensor {
public:
    static void init();
    static bool readData(float &temp, float &humi);
};

#endif // DHT_SENSOR_H
