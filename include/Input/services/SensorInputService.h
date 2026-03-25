#ifndef SENSOR_INPUT_SERVICE_H
#define SENSOR_INPUT_SERVICE_H

#include "Common/Events.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

class SensorInputService {
public:
  static void task_sensor_poll(void *param);
  static void readSensors(QueueHandle_t *qProcessing);
};

#endif
