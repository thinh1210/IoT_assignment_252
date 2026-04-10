#include "Main_FSM/modes/AccessPointMode.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "services/WifiService.h"
#include "config.h"

extern float globalTemp;
extern float globalHumi;

static const char *TAG = "AccessPointMode";

void AccessPointMode::enter() {
  ESP_LOGI(TAG, "Initializing Processing AccessPoint Mode Worker...");
  WifiService::startAccessPoint();
  if (task_button_handle != NULL) vTaskResume(task_button_handle);
  if (task_sensor_handle != NULL) vTaskSuspend(task_sensor_handle); // Don't need sensor in AP mode really
  xTaskCreate(AccessPointMode::run, "proc_ap_task", 8192, NULL, 4, &task_ap_mode_handle);
}

void AccessPointMode::exit() {
  if (task_ap_mode_handle != NULL) {
    vTaskDelete(task_ap_mode_handle);
    task_ap_mode_handle = NULL;
  }
}

void AccessPointMode::run(void *param) {
  ESP_LOGI(TAG, "AccessPoint Mode Sub-task: Config Monitor started.");

  uint32_t lastBroadCast = 0;
  while (1) {
    uint32_t now = millis();
    if (now - lastBroadCast >= 3000) {
      WifiService::broadcastTelemetry(globalTemp, globalHumi);
      lastBroadCast = now;
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
    ESP_LOGI(TAG, "[AccessPoint Sub-task] Monitoring config portal...");
  }
}
