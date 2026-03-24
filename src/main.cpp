#include <Arduino.h>
#include "TaskManager.h"
#include "Processing/ProcessingLayer.h"
#include "Processing/WifiModeManager.h"
#include "config.h"

// Note: This main.cpp tests the full system integration with AccessPoint renaming.
// You can switch modes via the Premium Web Interface (Chuyển Chế Độ button)
// or by holding the button on the device.

static const char *TAG = "MainApp";

// Global variables defined here
float globalTemp = 0.0f;
float globalHumi = 0.0f;

void setup() {
    Serial.begin(115200);
    delay(2000);
    ESP_LOGI(TAG, "--- Project IoT: AccessPoint Mode Test ---");

    // 1. Initialize Queue for Inter-layer communication
    static QueueHandle_t qInputToProcessing = xQueueCreate(QUEUE_SIZE, sizeof(SystemEvent));

    // 2. Initialize Layers
    ProcessingLayer::init(&qInputToProcessing);
    InputLayer::init(&qInputToProcessing);

    // 3. Initialize Mode Tasks (Normal & AccessPoint)
    TaskManager::initModes();

    // VI DU: Ep buộc vào chế độ AccessPoint trực tiếp từ main để test (nếu muốn)
    // Uncomment lines below to force start in AccessPoint mode
    /*
    ESP_LOGI(TAG, "Forcing AccessPoint Mode for testing...");
    ProcessingLayer::switchMode(SystemMode::ACCESSPOINT_MODE);
    */

    ESP_LOGI(TAG, "System running. Connect to 'ESP32' WiFi to access portal.");
}

void loop() {
    // The system is managed by FreeRTOS tasks initialized in TaskManager
    // No code needed here.
    vTaskDelay(pdMS_TO_TICKS(1000));
}
