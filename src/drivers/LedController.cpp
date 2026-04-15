#include "drivers/LedController.h"
#include "Common/AppLog.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char *TAG = "LedController";
SemaphoreHandle_t LedController::ledMutex = NULL;
bool LedController::ledState = false;

void LedController::init() {
    if (ledMutex == NULL) {
        ledMutex = xSemaphoreCreateMutex();
    }
    
    if (GREEN_LED_GPIO != -1) {
        pinMode(GREEN_LED_GPIO, OUTPUT);
        digitalWrite(GREEN_LED_GPIO, LOW);
    }
    ledState = false;
    ESP_LOGI(TAG, "LED Controller Initialized on GPIO %d", GREEN_LED_GPIO);
}

void LedController::setState(bool on) {
    if (GREEN_LED_GPIO == -1 || ledMutex == NULL) return;

    if (xSemaphoreTake(ledMutex, portMAX_DELAY) == pdPASS) {
        digitalWrite(GREEN_LED_GPIO, on ? HIGH : LOW);
        ledState = on;
        xSemaphoreGive(ledMutex);
    }
}

void LedController::toggle() {
    setState(!isOn());
}

bool LedController::isOn() {
    bool currentState = ledState;
    if (ledMutex == NULL) {
        return currentState;
    }

    if (xSemaphoreTake(ledMutex, pdMS_TO_TICKS(20)) == pdPASS) {
        currentState = ledState;
        xSemaphoreGive(ledMutex);
    }
    return currentState;
}

void LedController::blink(uint32_t delayMs, uint32_t times) {
    if (GREEN_LED_GPIO == -1) return;
    if (ledMutex == NULL) return;

    // Chờ lấy khóa (Mutex). Nếu có task khác đang blink thì task này sẽ đợi.
    if (xSemaphoreTake(ledMutex, portMAX_DELAY) == pdPASS) {
        for (uint32_t i = 0; i < times; i++) {
            digitalWrite(GREEN_LED_GPIO, HIGH);
            ledState = true;
            vTaskDelay(pdMS_TO_TICKS(delayMs));
            digitalWrite(GREEN_LED_GPIO, LOW);
            ledState = false;
            vTaskDelay(pdMS_TO_TICKS(delayMs));
        }
        // Trả khóa sau khi blink xong
        xSemaphoreGive(ledMutex);
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
