#include "Processing/LedController.h"
#include "esp_log.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char *TAG = "LedController";

void LedController::init() {
    if (GREEN_LED_GPIO != -1) {
        pinMode(GREEN_LED_GPIO, OUTPUT);
        digitalWrite(GREEN_LED_GPIO, LOW);
    }
    ESP_LOGI(TAG, "LED Controller Initialized on GPIO %d", GREEN_LED_GPIO);
}

void LedController::blink(uint32_t delayMs, uint32_t times) {
    if (GREEN_LED_GPIO == -1) return;
    
    for (uint32_t i = 0; i < times; i++) {
        digitalWrite(GREEN_LED_GPIO, HIGH);
        vTaskDelay(pdMS_TO_TICKS(delayMs));
        digitalWrite(GREEN_LED_GPIO, LOW);
        vTaskDelay(pdMS_TO_TICKS(delayMs));
    }
}

void LedController::blink1000ms_3times() {
    ESP_LOGI(TAG, "Blinking 1000ms 3 times");
    blink(1000, 3);
}

void LedController::blink500ms_3times() {
    ESP_LOGI(TAG, "Blinking 500ms 3 times");
    blink(500, 3);
}

void LedController::blink200ms_for_3s() {
    ESP_LOGI(TAG, "Blinking 200ms for 3 seconds");
    // 3 seconds total = 3000ms. Each full blink is 400ms (200 on, 200 off).
    // Times = 3000 / 400 = 7.5 -> rounded to 7 times.
    blink(200, 7);
}
