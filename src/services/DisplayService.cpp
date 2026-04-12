#include "services/DisplayService.h"
#include "drivers/OLEDDisplay.h"
#include "esp_log.h"
#include "config.h" 

static const char *TAG = "DisplayService";

void DisplayService::init() {
    ESP_LOGI(TAG, "Initializing synchronous DisplayService...");
    OLEDDisplay::getInstance().init();
}

void DisplayService::showNormalMode(float t, float h) {
    OLEDDisplay& oled = OLEDDisplay::getInstance();
    oled.clear();
    oled.drawTelemetryPage(t, h);
    oled.render();
}

void DisplayService::showAPMode(int signalLevel) {
    OLEDDisplay& oled = OLEDDisplay::getInstance();
    oled.clear();
    oled.drawAPPage(signalLevel);
    oled.render();
}

void DisplayService::showSuccessBlocking(uint32_t duration_ms) {
    OLEDDisplay& oled = OLEDDisplay::getInstance();
    oled.clear();
    oled.drawSuccessPage();
    oled.render();
    
    // Block the sequence for a few seconds as requested
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
}

void DisplayService::showLoadingStep(int step) {
    OLEDDisplay& oled = OLEDDisplay::getInstance();
    oled.clear();
    oled.drawLoadingPage(step);
    oled.render();
}
