#include "services/DisplayService.h"
#include "drivers/OLEDDisplay.h"
#include "config.h"
#include "esp_log.h"

static const char *TAG = "DisplayService";

TaskHandle_t DisplayService::task_display_handle = NULL;

void DisplayService::init() {
  if (task_display_handle != NULL) return;

  OLEDDisplay::getInstance().init();
  xTaskCreate(task_display_loop, "oled_display", 4096, NULL, 3,
              &task_display_handle);
  ESP_LOGI(TAG, "OLED display task started.");
}

void DisplayService::task_display_loop(void *param) {
  OLEDDisplay &display = OLEDDisplay::getInstance();
  uint32_t renderCount = 0;

  ESP_LOGI(TAG, "OLED display loop started. Refresh interval = 1000 ms");

  while (1) {
    const float temp = globalTemp;
    const float humi = globalHumi;

    display.clear();
    display.drawTelemetryPage(temp, humi);
    display.render();
    renderCount++;

    if (renderCount % 10 == 0) {
      ESP_LOGI(TAG, "OLED heartbeat -> showing T=%.1f C, H=%.0f %%",
               temp, humi);
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
