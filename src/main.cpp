#include "Processing/ProcessingLayer.h"
#include "Processing/WifiModeManager.h"
#include "TaskManager.h"
#include "config.h"
#include <Arduino.h>
#include <Preferences.h>

// Note: This main.cpp tests the full system integration with AccessPoint
// renaming. You can switch modes via the Premium Web Interface (Chuyển Chế Độ
// button) or by holding the button on the device.

static const char *TAG = "MainApp";

// Global variables defined here
float globalTemp = 0.0f;
float globalHumi = 0.0f;
ConfigData globalConfig;

void setup() {
  Serial.begin(115200);
  delay(2000);
  ESP_LOGI(TAG, "--- Project IoT: AccessPoint Mode Test ---");

  // 1. Initialize Queue for Inter-layer communication
  static QueueHandle_t qInputToProcessing =
      xQueueCreate(QUEUE_SIZE, sizeof(SystemEvent));

  // 2. Initialize Layers
  ProcessingLayer::init(&qInputToProcessing);
  InputLayer::init(&qInputToProcessing);

  // 3. Initialize Mode Tasks (Normal & AccessPoint)
  TaskManager::initModes();
}

void loop() {
  // The system is managed by FreeRTOS tasks initialized in TaskManager
  // No code needed here.
  vTaskDelay(pdMS_TO_TICKS(1000));
}
