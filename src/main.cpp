#include "Main_FSM/Main_FSM.h"
#include "services/DisplayService.h"
#include "services/WifiService.h"
#include "TaskManager.h"
#include "Common/AppLog.h"
#include "config.h"
#include <Arduino.h>
#include <Preferences.h>

static const char *TAG = "MainApp";

// Global variables defined here
volatile float globalTemp = 0.0f;
volatile float globalHumi = 0.0f;
ConfigData globalConfig;
volatile int globalPlantCareAction = 0;
volatile float globalPlantCareConfidence = 0.0f;
volatile bool globalPlantCareReady = false;

// FreeRTOS Handles Global Definitions
TaskHandle_t task_sys_manager_handle = NULL;
TaskHandle_t task_input_manager_handle = NULL;
TaskHandle_t task_manager_handle = NULL;
TaskHandle_t task_normal_mode_handle = NULL;
TaskHandle_t task_manual_mode_handle = NULL;
TaskHandle_t task_ap_mode_handle = NULL;
TaskHandle_t task_accesspoint_mode_handle = NULL;
TaskHandle_t task_button_handle = NULL;
TaskHandle_t task_sensor_handle = NULL;
TaskHandle_t task_ir_remote_handle = NULL;
TaskHandle_t task_plant_care_handle = NULL;

SemaphoreHandle_t btnSemaphore = NULL;
SemaphoreHandle_t sensorSemaphore = NULL;

void setup() {
  Serial.begin(115200);
  AppLog::init();
  ESP_LOGI(TAG, "USB CDC ready");
  ESP_LOGI(TAG, "--- Project IoT: System Architecture Restored ---");
  delay(200);

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
