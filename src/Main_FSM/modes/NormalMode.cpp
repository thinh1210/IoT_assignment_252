#include "Main_FSM/modes/NormalMode.h"
#include "services/WifiService.h"
#include "config.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern float globalTemp;
extern float globalHumi;

static const char *TAG = "NormalMode";

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
      WifiService::broadcastTelemetry(globalTemp, globalHumi);
      lastTeleSend = now;
    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
