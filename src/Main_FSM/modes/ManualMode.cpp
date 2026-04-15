#include "Main_FSM/modes/ManualMode.h"

#include "config.h"
#include "Common/AppLog.h"
#include "freertos/FreeRTOS.h"
#include "services/WifiService.h"

static const char *TAG = "ManualMode";

void ManualMode::run(void *param) {
  uint32_t lastTeleSend = 0;
  ESP_LOGI(TAG, "Manual Mode Sub-task: Connectivity & Telemetry started.");

  while (1) {
    const uint32_t now = millis();

    WifiService::maintainConnections();

    if (now - lastTeleSend >= 5000) {
      WifiService::sendTelemetry(globalTemp, globalHumi);
      lastTeleSend = now;
    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
