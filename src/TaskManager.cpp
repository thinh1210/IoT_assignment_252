#include "TaskManager.h"
#include "Input/InputLayer.h"
#include "Processing/ProcessingLayer.h"
#include "esp_log.h"

static const char *TAG = "TaskManager";

void TaskManager::initModes() {
    ESP_LOGI(TAG, "Starting System in Normal Mode...");
    
    // Switch to initial mode (This will call initNormalMode internally in each layer)
    InputLayer::switchMode(SystemMode::NORMAL_MODE);
    ProcessingLayer::switchMode(SystemMode::NORMAL_MODE);
}
