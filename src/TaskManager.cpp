#include "TaskManager.h"
#include "Input/InputLayer.h"
#include "Main_FSM/Main_FSM.h"
#include "Common/AppLog.h"

static const char *TAG = "TaskManager";

void TaskManager::initModes() {
    ESP_LOGI(TAG, "Starting System in Normal Mode...");

    // On cold boot, Main_FSM already starts in NORMAL state, so switchMode()
    // would return early and skip creating the normal worker task.
    InputLayer::switchMode(SystemMode::NORMAL_MODE);
    Main_FSM::initNormalMode();
}
