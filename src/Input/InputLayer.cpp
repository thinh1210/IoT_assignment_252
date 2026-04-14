#include "Input/InputLayer.h"
#include "drivers/DHTSensor.h"
#include "Input/services/ButtonInputService.h"
#include "Input/services/IrRemoteInputService.h"
#include "Input/services/SensorInputService.h"
#include "esp_log.h"

static const char *TAG = "InputLayer";

// Define static variables
QueueHandle_t *InputLayer::qProcessing = NULL;
ButtonHandler *InputLayer::btnHandler = NULL;
SystemMode InputLayer::currentMode = SystemMode::NORMAL_MODE;

void InputLayer::init(QueueHandle_t *qProc) {
  ESP_LOGI(TAG, "General InputLayer Initialization...");
  qProcessing = qProc;

  // 1. Create counting semaphore to receive button events
  btnSemaphore = xSemaphoreCreateCounting(20, 0);
  ESP_LOGI(TAG, "Button semaphore created: %p", btnSemaphore);

  // 2. Initialize ButtonHandler
  btnHandler = new ButtonHandler();
  btnHandler->setSemaphore(btnSemaphore);


  if (BUTTON_2_GPIO != -1) {
    btnHandler->addButton(BUTTON_2_GPIO, 2000, true);
  }
  btnHandler->begin();

  if (GREEN_LED_GPIO != -1)
    pinMode(GREEN_LED_GPIO, OUTPUT);

  // Initialize Sensors
  DHTSensor::init();

  // Create sensor semaphore
  sensorSemaphore = xSemaphoreCreateBinary();
  ESP_LOGI(TAG, "Sensor semaphore created: %p", sensorSemaphore);

  // 3. Start Sub-Tasks for continuous polling
  if (xTaskCreate(ButtonInputService::task_button_poll, "btn_poll", 4096,
                  btnHandler, 5, &task_button_handle) == pdPASS) {
    ESP_LOGI(TAG, "Task created: btn_poll (%p)", task_button_handle);
  } else {
    ESP_LOGE(TAG, "Failed to create task: btn_poll");
  }

  if (xTaskCreate(SensorInputService::task_sensor_poll, "sens_poll", 4096,
                  sensorSemaphore, 4, &task_sensor_handle) == pdPASS) {
    ESP_LOGI(TAG, "Task created: sens_poll (%p)", task_sensor_handle);
  } else {
    ESP_LOGE(TAG, "Failed to create task: sens_poll");
  }

  if (xTaskCreate(IrRemoteInputService::task_ir_remote_poll, "ir_remote_poll",
                  6144, qProcessing, 4, &task_ir_remote_handle) == pdPASS) {
    ESP_LOGI(TAG, "Task created: ir_remote_poll (%p)", task_ir_remote_handle);
  } else {
    ESP_LOGE(TAG, "Failed to create task: ir_remote_poll");
  }

  // 4. Start Persistent Input Manager
  if (xTaskCreate(task_manager, "input_manager", 4096, NULL, 6,
                  &task_manager_handle) == pdPASS) {
    ESP_LOGI(TAG, "Task created: input_manager (%p)", task_manager_handle);
  } else {
    ESP_LOGE(TAG, "Failed to create task: input_manager");
  }

  ESP_LOGI(TAG, "InputLayer General Init successful.");
}

void InputLayer::task_manager(void *param) {
  ESP_LOGI(TAG, "Input Manager Task started. Waiting for button/sensor events...");
  while (1) {
    if (xSemaphoreTake(btnSemaphore, pdMS_TO_TICKS(10)) == pdPASS) {
      ButtonInputService::handleButtonEvents(btnHandler, qProcessing);
    }
    
    if (xSemaphoreTake(sensorSemaphore, pdMS_TO_TICKS(10)) == pdPASS) {
      SensorInputService::readSensors(qProcessing);
    }
    
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void InputLayer::switchMode(SystemMode newMode) {
  if (currentMode == newMode) return;
  ESP_LOGI(TAG, "Switching Input Mode: %d -> %d", (int)currentMode, (int)newMode);
  currentMode = newMode;
}

void InputLayer::pushEvent(const SystemEvent &event) {
  if (qProcessing != NULL) {
    xQueueSend(*qProcessing, &event, 0);
  }
}
