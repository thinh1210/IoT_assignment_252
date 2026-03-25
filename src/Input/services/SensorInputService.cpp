#include "Input/services/SensorInputService.h"
#include "drivers/DHTSensor.h"
#include "config.h"
#include "esp_log.h"

void SensorInputService::task_sensor_poll(void *param) {
  SemaphoreHandle_t syncSem = (SemaphoreHandle_t)param;
  while (1) {
    vTaskDelay(pdMS_TO_TICKS(3000));
    if (syncSem != NULL) {
      xSemaphoreGive(syncSem);
    }
  }
}

void SensorInputService::readSensors(QueueHandle_t *qProcessing) {
  float temp, humi;
  if (DHTSensor::readData(temp, humi)) {
    globalTemp = temp;
    globalHumi = humi;

    SystemEvent event;
    event.type = EventType::SENSOR_DATA_READY;
    if (qProcessing != NULL) {
      xQueueSend(*qProcessing, &event, 0);
    }
  }
}
