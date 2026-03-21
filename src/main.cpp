#include "Common/Events.h"
#include "Input/InputLayer.h"
#include "Processing/ProcessingLayer.h"
#include "TaskManager.h"
#include "config.h"
#include "esp_log.h"
#include <Arduino.h>

static const char *TAG = "Main";

// Communication Queue
QueueHandle_t mainQueue = NULL;

// Global variables definition
float globalTemp = 0.0f;
float globalHumi = 0.0f;

#include "Input/DHTSensor.h"

void setup() {
  Serial.begin(115200);
  vTaskDelay(pdMS_TO_TICKS(1000));
  ESP_LOGI(TAG, "System Starting in TEST MODE (DHT11 Only)...");

  // Create central queue
  mainQueue = xQueueCreate(QUEUE_SIZE, sizeof(SystemEvent));

  if (mainQueue != NULL) {
    ESP_LOGI(TAG, "Main queue created successfully.");

    // Initialize layers
    InputLayer::init(&mainQueue);
    ProcessingLayer::init(&mainQueue);

    // Initialize Mode sub-tasks via TaskManager
    TaskManager::initModes();

    ESP_LOGI(TAG, "All initialization finished. Deleting setup task.");
  } else {
    ESP_LOGE(TAG, "Failed to create main queue!");
  }

  // // Khởi tạo DHT Sensor
  // DHTSensor::init();
}

void loop() {
  // float temp, humi;
  // if (DHTSensor::readData(temp, humi)) {
  //   ESP_LOGI(TAG, "Dữ liệu DHT11: Nhiệt độ = %.1f C, Độ ẩm = %.1f %%", temp,
  //            humi);
  // } else {
  //   ESP_LOGE(TAG, "Không đọc được dữ liệu DHT11!");
  // }

  // vTaskDelay(pdMS_TO_TICKS(2000)); // Đọc mỗi 2 giây
}