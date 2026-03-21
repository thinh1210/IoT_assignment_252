#include "Processing/ProcessingLayer.h"
#include "Processing/LedController.h"
#include "Processing/NeonController.h"
#include "TaskManager.h"
#include "esp_log.h"

static const char *TAG = "ProcessingLayer";

// Define static variables
QueueHandle_t *ProcessingLayer::qInput = NULL;
TaskHandle_t ProcessingLayer::modeATaskHandle = NULL;
TaskHandle_t ProcessingLayer::modeBTaskHandle = NULL;
SystemMode ProcessingLayer::currentMode = SystemMode::MODE_A;

void ProcessingLayer::init(QueueHandle_t *qIn) {
  ESP_LOGI(TAG, "Entering %s", __FUNCTION__);

  LedController::init();
  NeonController::init(); // Initialize Neon Lamp
  qInput = qIn;
  ESP_LOGI(TAG, "ProcessingLayer initialized.");
}

void ProcessingLayer::initModeA() {
  ESP_LOGI(TAG, "Initialize Processing Mode A Task.");
  xTaskCreate(taskModeALoop, "ProcTaskModeA", 4096, NULL, 6, &modeATaskHandle);
}

void ProcessingLayer::initModeB() {
  ESP_LOGI(TAG, "Initialize Processing Mode B Task.");
  xTaskCreate(taskModeBLoop, "ProcTaskModeB", 4096, NULL, 6, &modeBTaskHandle);
  if (modeBTaskHandle != NULL) {
    vTaskSuspend(modeBTaskHandle);
  }
}

void ProcessingLayer::switchMode(SystemMode newMode) {
  if (currentMode == newMode)
    return;

  ESP_LOGI(TAG, "Processing switching Mode: %d -> %d", (int)currentMode,
           (int)newMode);

  switch (newMode) {
  case SystemMode::MODE_A:
    if (modeBTaskHandle != NULL)
      vTaskSuspend(modeBTaskHandle);
    if (modeATaskHandle != NULL)
      vTaskResume(modeATaskHandle);
    break;
  case SystemMode::MODE_B:
    if (modeATaskHandle != NULL)
      vTaskSuspend(modeATaskHandle);
    if (modeBTaskHandle != NULL)
      vTaskResume(modeBTaskHandle);
    break;
  }

  currentMode = newMode;
}

void ProcessingLayer::handleEvent(SystemEvent event) {
  switch (event.type) {
  case EventType::SENSOR_DATA_READY:
    ESP_LOGI(TAG, "Processing: Temp=%.1f C, Humi=%.1f %%", globalTemp,
             globalHumi);
    break;
  case EventType::BUTTON_PRESSED:
    // ESP_LOGI(TAG, "Processing: Button Pressed - Blinking 200ms for 3s");
    // LedController::blink200ms_for_3s();
    ESP_LOGI(TAG, "Processing: Button Pressed - Changing Neon Color");
    NeonController::setNextColor(); // Change neon lamp color
    break;
  case EventType::MODE_CHANGE: {
    ESP_LOGI(TAG, "Processing: MODE_CHANGE Event Received.");
    SystemMode nextMode = (currentMode == SystemMode::MODE_A)
                              ? SystemMode::MODE_B
                              : SystemMode::MODE_A;

    ProcessingLayer::switchMode(nextMode);
    InputLayer::switchMode(nextMode);
    break;
  }
  default:
    break;
  }
}

void ProcessingLayer::taskModeALoop(void *param) {
  SystemEvent event;
  while (1) {
    // Xử lý các event trong queue trước nếu có (non-blocking)
    if (qInput != NULL) {
      while (xQueueReceive(*qInput, &event, 0) == pdPASS) {
        ProcessingLayer::handleEvent(event);
      }
    }

    vTaskDelay(pdMS_TO_TICKS(50)); // Khoảng ngh ỉ nhỏ
  }
}

void ProcessingLayer::taskModeBLoop(void *param) {
  SystemEvent event;
  while (1) {
    // Xử lý các event trong queue trước nếu có (non-blocking)
    if (qInput != NULL) {
      while (xQueueReceive(*qInput, &event, 0) == pdPASS) {
        ProcessingLayer::handleEvent(event);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(50)); // Khoảng nghỉ nhỏ
  }
}
