#include "Main_FSM/modes/NormalMode.h"
#include "config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "services/DisplayService.h"
#include "services/WifiService.h"

static const char *TAG = "NormalMode";

void NormalMode::enter() {
  ESP_LOGI(TAG, "Initializing Processing Normal Mode Worker...");
  WifiService::startClient();
  DisplayService::showNormalMode(globalTemp, globalHumi);
  if (task_button_handle != NULL)
    vTaskResume(task_button_handle);
  if (task_sensor_handle != NULL)
    vTaskResume(task_sensor_handle);
  xTaskCreate(NormalMode::run, "proc_normal_task", 8192, NULL, 4,
              &task_normal_mode_handle);
}

void NormalMode::exit() {
  if (task_normal_mode_handle != NULL) {
    vTaskDelete(task_normal_mode_handle);
    task_normal_mode_handle = NULL;
  }
}

void NormalMode::run(void *param) {
  uint32_t lastTeleSend = 0;
  ESP_LOGI(TAG, "Normal Mode Sub-task: Connectivity & Telemetry started.");

  while (1) {
    uint32_t now = millis();

    // Maintain long-running connections (thingsboard, wifi)
    WifiService::maintainConnections();

    // Periodic tasks for this mode
    if (now - lastTeleSend >= 5000) {
      WifiService::sendTelemetry(globalTemp, globalHumi);
      lastTeleSend = now;
    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
