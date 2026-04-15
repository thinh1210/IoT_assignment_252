#include "services/DisplayService.h"
#include "drivers/OLEDDisplay.h"
#include "config.h"
#include "Common/AppLog.h"
#include "services/ManualControlService.h"

static const char *TAG = "DisplayService";

TaskHandle_t DisplayService::task_display_handle = NULL;
volatile DisplayService::DisplayView DisplayService::currentView =
    DisplayService::DisplayView::NORMAL;
volatile int DisplayService::apSignalLevel = 0;

namespace {

bool hasSensorData(float humi) {
  return humi > 0.0f;
}

const char *resolveStatusText(float temp, float humi) {
  if (!hasSensorData(humi)) {
    return "STATUS: NO SENSOR";
  }
  if (temp >= ENV_TEMP_CRITICAL_THRESHOLD &&
      humi <= ENV_HUMI_CRITICAL_LOW_THRESHOLD) {
    return "ALERT: HOT + DRY";
  }
  if (temp >= ENV_TEMP_CRITICAL_THRESHOLD) {
    return "ALERT: TOO HOT";
  }
  if (humi <= ENV_HUMI_CRITICAL_LOW_THRESHOLD) {
    return "ALERT: TOO DRY";
  }
  if (humi >= ENV_HUMI_CRITICAL_HIGH_THRESHOLD) {
    return "ALERT: TOO HUMID";
  }
  if (temp >= ENV_TEMP_WARNING_THRESHOLD &&
      humi <= ENV_HUMI_WARNING_LOW_THRESHOLD) {
    return "WARN: HOT + DRY";
  }
  if (temp >= ENV_TEMP_WARNING_THRESHOLD) {
    return "WARN: SLIGHT HOT";
  }
  if (humi <= ENV_HUMI_WARNING_LOW_THRESHOLD) {
    return "WARN: SLIGHT DRY";
  }
  return "STATUS: NORMAL";
}

} // namespace

void DisplayService::init() {
  if (task_display_handle != NULL) return;

  OLEDDisplay::getInstance().init();
  OLEDDisplay::getInstance().playWelcomeAnimation("Khoa");
  xTaskCreate(task_display_loop, "oled_display", 4096, NULL, 3,
              &task_display_handle);
  ESP_LOGI(TAG, "OLED display task started.");
}

void DisplayService::showNormalMode(float temp, float humi) {
  currentView = DisplayView::NORMAL;
  ESP_LOGI(TAG, "Display switched to NORMAL mode view. T=%.1f C, H=%.0f %%",
           temp, humi);
}

void DisplayService::showAPMode(int level) {
  apSignalLevel = level;
  currentView = DisplayView::AP;
  ESP_LOGI(TAG, "Display switched to AP mode view. level=%d", level);
}

void DisplayService::showManualMode() {
  currentView = DisplayView::MANUAL;
  ESP_LOGI(TAG, "Display switched to MANUAL mode view.");
}

void DisplayService::task_display_loop(void *param) {
  OLEDDisplay &display = OLEDDisplay::getInstance();
  uint32_t renderCount = 0;

  ESP_LOGI(TAG, "OLED display loop started. Refresh interval = 1000 ms");

  while (1) {
    const float temp = globalTemp;
    const float humi = globalHumi;
    const DisplayView view = currentView;
    char manualStatus[24] = {0};
    const char *statusText = resolveStatusText(temp, humi);

    display.clear();
    if (view == DisplayView::AP) {
      display.drawAPPage(apSignalLevel, statusText);
    } else if (view == DisplayView::MANUAL) {
      ManualControlService::formatStatusLine(manualStatus, sizeof(manualStatus));
      display.drawTelemetryPage(temp, humi, "MODE: MANUAL", manualStatus);
    } else {
      display.drawTelemetryPage(temp, humi, "MODE: NORMAL", statusText);
    }
    display.render();
    renderCount++;

    if (renderCount % 10 == 0) {
      if (view == DisplayView::AP) {
        ESP_LOGI(TAG, "OLED heartbeat -> AP mode, level=%d, %s",
                 apSignalLevel, statusText);
      } else if (view == DisplayView::MANUAL) {
        ESP_LOGI(TAG,
                 "OLED heartbeat -> mode=MANUAL, T=%.1f C, H=%.0f %%, %s",
                 temp, humi, manualStatus);
      } else {
        ESP_LOGI(TAG, "OLED heartbeat -> mode=NORMAL, T=%.1f C, H=%.0f %%, %s",
                 temp, humi, statusText);
      }
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
