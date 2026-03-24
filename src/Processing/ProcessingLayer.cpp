#include "Processing/ProcessingLayer.h"
#include "Processing/LedController.h"
#include "Processing/NeonController.h"
#include "Processing/WifiModeManager.h"
#include "TaskManager.h"
#include "esp_log.h"

static const char *TAG = "ProcessingLayer";

// Define static variables
QueueHandle_t *ProcessingLayer::qInput = NULL;
TaskHandle_t ProcessingLayer::task_manager_handle = NULL;
TaskHandle_t ProcessingLayer::task_normal_mode_handle = NULL;
TaskHandle_t ProcessingLayer::task_accesspoint_mode_handle = NULL;
SystemMode ProcessingLayer::currentMode = SystemMode::NORMAL_MODE;
String ProcessingLayer::deviceUID = "DEV-0001";
ConfigData ProcessingLayer::currentConfig;
Preferences ProcessingLayer::preferences;

/**
 * @brief Initialize the Processing Layer
 */
void ProcessingLayer::init(QueueHandle_t *qIn) {
  ESP_LOGI(TAG, "Initializing Processing Layer...");

  LedController::init();
  NeonController::init();
  loadConfig(); // Load config before initializing WifiModeManager
  WifiModeManager::init();
  qInput = qIn;

  // Rule 3.3.2: Processing layer starts the Manager Task which coordinates
  // everything
  ProcessingLayer::startManager();

  ESP_LOGI(TAG, "Processing Layer ready.");
}

void ProcessingLayer::startManager() {
  xTaskCreate(task_manager, "task_manager", 8192, NULL, 5,
              &task_manager_handle);
}

void ProcessingLayer::initNormalMode() {
  // Normal Mode can have multiple sub-tasks. For now: 1 worker.
  xTaskCreate(task_normal_mode, "task_normal_mode", 8192, NULL, 4,
              &task_normal_mode_handle);
}

void ProcessingLayer::initAccessPointMode() {
  // AccessPoint Mode can have multiple sub-tasks. For now: 1 worker.
  xTaskCreate(task_accesspoint_mode, "task_accesspoint_mode", 8192, NULL, 4,
              &task_accesspoint_mode_handle);
  if (task_accesspoint_mode_handle != NULL) {
    vTaskSuspend(task_accesspoint_mode_handle);
  }
}

/**
 * @brief Manager Task: Handle Input from queue and manage task states of all
 * layers
 */
void ProcessingLayer::task_manager(void *param) {
  SystemEvent event;
  ESP_LOGI(TAG, "Manager Task started. Listening for Input events...");

  while (1) {
    // Wait for event from Input Layer (Rule 4.2)
    if (qInput != NULL &&
        xQueueReceive(*qInput, &event, pdMS_TO_TICKS(100)) == pdPASS) {
      ProcessingLayer::handleEvent(event);
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

/**
 * @brief Orchestrate mode switching by suspending/resuming tasks across layers
 */
void ProcessingLayer::switchMode(SystemMode newMode) {
  if (currentMode == newMode)
    return;

  ESP_LOGI(TAG, "Mode Transition: %s -> %s",
           (currentMode == SystemMode::NORMAL_MODE ? "NORMAL" : "ACCESSPOINT"),
           (newMode == SystemMode::NORMAL_MODE ? "NORMAL" : "ACCESSPOINT"));

  // 1. Suspend current mode tasks (Processing + Input)
  if (currentMode == SystemMode::NORMAL_MODE) {
    if (task_normal_mode_handle != NULL)
      vTaskSuspend(task_normal_mode_handle);
    // Add additional normal sub-tasks suspension here
  } else if (currentMode == SystemMode::ACCESSPOINT_MODE) {
    if (task_accesspoint_mode_handle != NULL)
      vTaskSuspend(task_accesspoint_mode_handle);
    // Add additional accesspoint sub-tasks suspension here
  }

  // 2. Perform Physical/Network transition
  if (newMode == SystemMode::ACCESSPOINT_MODE) {
    WifiModeManager::startAccessPoint();
  } else {
    WifiModeManager::startClient();
  }

  // 3. Resume new mode tasks (Processing + Input)
  if (newMode == SystemMode::NORMAL_MODE) {
    if (task_normal_mode_handle != NULL)
      vTaskResume(task_normal_mode_handle);
  } else if (newMode == SystemMode::ACCESSPOINT_MODE) {
    if (task_accesspoint_mode_handle != NULL)
      vTaskResume(task_accesspoint_mode_handle);
  }

  // 4. Delegate Input Layer mode switch (Coordination)
  InputLayer::switchMode(newMode);

  currentMode = newMode;
}

void ProcessingLayer::handleEvent(SystemEvent event) {
  switch (event.type) {
  case EventType::BUTTON_PRESSED:
    ESP_LOGI(TAG, "Event: Button Pressed. Action: Change Neon color.");
    NeonController::setNextColor();
    break;

  case EventType::MODE_CHANGE: {
    SystemMode nextMode = (currentMode == SystemMode::NORMAL_MODE)
                              ? SystemMode::ACCESSPOINT_MODE
                              : SystemMode::NORMAL_MODE;
    ProcessingLayer::switchMode(nextMode);
    break;
  }

  case EventType::SENSOR_DATA_READY:
    // Telemetry is handled in the normal worker task
    break;

  default:
    break;
  }
}

// --- SUB-TASKS FOR NORMAL MODE ---

void ProcessingLayer::task_normal_mode(void *param) {
  uint32_t lastTeleSend = 0;
  ESP_LOGI(TAG, "Normal Mode Sub-task: Connectivity & Telemetry started.");

  while (1) {
    uint32_t now = millis();

    // Maintain long-running connections (thingsboard, wifi)
    WifiModeManager::maintainConnections();

    // Periodic tasks for this mode
    if (now - lastTeleSend >= 5000) {
      WifiModeManager::sendTelemetry(globalTemp, globalHumi);
      lastTeleSend = now;
    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// --- SUB-TASKS FOR ACCESSPOINT MODE ---

void ProcessingLayer::task_accesspoint_mode(void *param) {
  ESP_LOGI(TAG, "AccessPoint Mode Sub-task: Config Monitor started.");

  while (1) {
    // Specialized tasks for AccessPoint mode: e.g. check for configuration
    // completion, LED blinking patterns for AP mode, or monitoring portal
    // status.

    vTaskDelay(pdMS_TO_TICKS(2000));
    ESP_LOGI(TAG, "[AccessPoint Sub-task] Monitoring config portal...");
  }
}

void ProcessingLayer::updateConfig(const ConfigData &newConfig) {
  currentConfig = newConfig;
  saveConfig();
}

void ProcessingLayer::saveConfig() {
  preferences.begin("device_conf", false);
  preferences.putString("wifi_ssid", currentConfig.wifi_ssid);
  preferences.putString("wifi_pass", currentConfig.wifi_pass);
  preferences.putString("mqtt_server", currentConfig.mqtt_server);
  preferences.putInt("mqtt_port", currentConfig.mqtt_port);
  preferences.putString("mqtt_user", currentConfig.mqtt_user);
  preferences.putString("mqtt_pass", currentConfig.mqtt_pass);
  preferences.putString("key_url", currentConfig.key_exchange_url);
  preferences.end();
  ESP_LOGI(TAG, "Configuration saved to flash.");
}

void ProcessingLayer::loadConfig() {
  preferences.begin("device_conf", false);
  deviceUID = preferences.getString("uid", "DEV-0001");
  currentConfig.wifi_ssid = preferences.getString("wifi_ssid", WIFI_SSID);
  currentConfig.wifi_pass = preferences.getString("wifi_pass", WIFI_PASSWORD);
  currentConfig.mqtt_server =
      preferences.getString("mqtt_server", "192.168.1.10");
  currentConfig.mqtt_port = preferences.getInt("mqtt_port", 1883);
  currentConfig.mqtt_user = preferences.getString("mqtt_user", "");
  currentConfig.mqtt_pass = preferences.getString("mqtt_pass", "");
  currentConfig.key_exchange_url =
      preferences.getString("key_url", "http://192.168.1.10:8000/exchange");
  preferences.end();
  ESP_LOGI(TAG, "Configuration loaded from flash.");
}
