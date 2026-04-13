#include "Main_FSM/Main_FSM.h"
#include "services/DisplayService.h"
#include "services/WifiService.h"
#include "TaskManager.h"
#include "config.h"
#include <Arduino.h>
#include <Preferences.h>

static const char *TAG = "MainApp";

// Global variables defined here
float globalTemp = 0.0f;
float globalHumi = 0.0f;
ConfigData globalConfig;

void setup() {
  Serial.begin(115200);
  ESP_LOGI(TAG, "--- Project IoT: System Architecture Restored ---");
  delay(2000);

  // 1. Initialize Queue for Inter-layer communication
  static QueueHandle_t qInputToProcessing =
      xQueueCreate(QUEUE_SIZE, sizeof(SystemEvent));

  // 2. Initialize Layers
  Main_FSM::init(&qInputToProcessing);
  InputLayer::init(&qInputToProcessing);
  DisplayService::init();

  // 3. Initialize Mode Tasks (Normal & AccessPoint)
  TaskManager::initModes();
}

void loop() {
  // The system is managed by FreeRTOS tasks initialized in TaskManager
  vTaskDelay(pdMS_TO_TICKS(1000));
}
