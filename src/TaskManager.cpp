#include "TaskManager.h"
#include "Input/InputLayer.h"
#include "Main_FSM/Main_FSM.h"
#include "esp_log.h"

static const char *TAG = "TaskManager";

void TaskManager::initModes() {
    ESP_LOGI(TAG, "Starting System in Normal Mode...");
    
    // Switch to initial mode (This will call initNormalMode internally in each layer)
    InputLayer::switchMode(SystemMode::NORMAL_MODE);
    Main_FSM::switchMode(SystemMode::NORMAL_MODE);
}
