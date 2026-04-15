#include "Input/services/SensorInputService.h"
#include "drivers/DHTSensor.h"
#include "config.h"
#include "Common/AppLog.h"

static const char *TAG = "SensorInputSvc";

void SensorInputService::task_sensor_poll(void *param) {
  SemaphoreHandle_t syncSem = (SemaphoreHandle_t)param;
  ESP_LOGI(TAG, "Sensor poll task started. Interval = %d ms", READ_INTERVAL);
  while (1) {
    vTaskDelay(pdMS_TO_TICKS(READ_INTERVAL));
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
    ESP_LOGI(TAG, "DHT read OK -> Temperature=%.1f C, Humidity=%.0f %%",
             temp, humi);

    SystemEvent event{};
    event.type = EventType::SENSOR_DATA_READY;
    if (qProcessing != NULL) {
      if (xQueueSend(*qProcessing, &event, 0) == pdPASS) {
        ESP_LOGI(TAG, "Queued SENSOR_DATA_READY event");
      } else {
        ESP_LOGW(TAG, "Failed to queue SENSOR_DATA_READY event");
      }
    }
  }
}
