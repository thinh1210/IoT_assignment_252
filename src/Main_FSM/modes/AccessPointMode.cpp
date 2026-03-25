#include "Main_FSM/modes/AccessPointMode.h"
#include "services/WifiService.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern float globalTemp;
extern float globalHumi;

static const char *TAG = "AccessPointMode";

void AccessPointMode::run(void *param) {
  ESP_LOGI(TAG, "AccessPoint Mode Sub-task: Config Monitor started.");

  uint32_t lastBroadCast = 0;
  while (1) {
    uint32_t now = millis();
    if (now - lastBroadCast >= 3000) {
      WifiService::broadcastTelemetry(globalTemp, globalHumi);
      lastBroadCast = now;
    }

    vTaskDelay(pdMS_TO_TICKS(2000));
    ESP_LOGI(TAG, "[AccessPoint Sub-task] Monitoring config portal...");
  }
}
