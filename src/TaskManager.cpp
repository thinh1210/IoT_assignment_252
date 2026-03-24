#include "TaskManager.h"
#include "Input/InputLayer.h"
#include "Processing/ProcessingLayer.h"
#include "esp_log.h"

static const char *TAG = "TaskManager";

void TaskManager::initModes() {
    ESP_LOGI(TAG, "Initializing Mode Tasks for all layers...");
    
    // Initialize mode tasks for all layers
    InputLayer::initNormalMode();
    InputLayer::initAccessPointMode();
    
    ProcessingLayer::initNormalMode();
    ProcessingLayer::initAccessPointMode();

    // Set initial default mode
    InputLayer::switchMode(SystemMode::NORMAL_MODE);
    ProcessingLayer::switchMode(SystemMode::NORMAL_MODE);
}
