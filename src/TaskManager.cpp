#include "TaskManager.h"
#include "Input/InputLayer.h"
#include "Processing/ProcessingLayer.h"
#include "esp_log.h"

static const char *TAG = "TaskManager";

void TaskManager::initModes() {
    ESP_LOGI(TAG, "Initializing Mode Tasks for all layers...");
    
    // Khởi tạo mode tasks từ các layer
    InputLayer::initModeA();
    InputLayer::initModeB();
    
    ProcessingLayer::initModeA();
    ProcessingLayer::initModeB();

    // Set default mode ban đầu trực tiếp
    
    InputLayer::switchMode(SystemMode::MODE_A);
    ProcessingLayer::switchMode(SystemMode::MODE_A);
}
