#include "Processing/ProcessingLayer.h"
#include "Input/DHTSensor.h"
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
Preferences ProcessingLayer::preferences;

/**
 * @brief Initialize the Processing Layer
 */
void ProcessingLayer::init(QueueHandle_t *qIn) {
  ESP_LOGI(TAG, "General ProcessingLayer Initialization...");
  qInput = qIn;

  LedController::init();
  NeonController::init();
  loadConfig(); // Rule: configuration belongs to processing layer

  // Initialize WiFi logic
  WifiModeManager::init();

  // Rule: State stored here
  currentMode = SystemMode::NORMAL_MODE;

  // Start the persistent Manager Task (Event Handler)
  ProcessingLayer::startManager();

  ESP_LOGI(TAG, "Processing Layer General Init ready.");
}

void ProcessingLayer::startManager() {
  xTaskCreate(task_manager, "system_manager", 8192, NULL, 5, &task_manager_handle);
}

void ProcessingLayer::initNormalMode() {
  deinitMode();
  ESP_LOGI(TAG, "Initializing Processing Normal Mode Worker...");
  xTaskCreate(task_normal_mode, "proc_normal_task", 8192, NULL, 4, &task_normal_mode_handle);
}

void ProcessingLayer::initAccessPointMode() {
  deinitMode();
  ESP_LOGI(TAG, "Initializing Processing AccessPoint Mode Worker...");
  xTaskCreate(task_accesspoint_mode, "proc_ap_task", 8192, NULL, 4, &task_accesspoint_mode_handle);
}

void ProcessingLayer::deinitMode() {
  if (task_normal_mode_handle != NULL) {
    vTaskDelete(task_normal_mode_handle);
    task_normal_mode_handle = NULL;
  }
  if (task_accesspoint_mode_handle != NULL) {
    vTaskDelete(task_accesspoint_mode_handle);
    task_accesspoint_mode_handle = NULL;
  }
  ESP_LOGI(TAG, "Worker tasks deleted.");
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
  if (currentMode == newMode) return;

  ESP_LOGI(TAG, "Mode Transition: %s -> %s",
           (currentMode == SystemMode::NORMAL_MODE ? "NORMAL" : "ACCESSPOINT"),
           (newMode == SystemMode::NORMAL_MODE ? "NORMAL" : "ACCESSPOINT"));

  // 1. Physical/Network transition
  if (newMode == SystemMode::ACCESSPOINT_MODE) {
    WifiModeManager::startAccessPoint();
  } else {
    WifiModeManager::startClient();
  }

  // 2. Delegate Input Layer mode switch (which handles its own task deletion)
  InputLayer::switchMode(newMode);

  // 3. Update own worker tasks
  if (newMode == SystemMode::NORMAL_MODE) {
    initNormalMode();
  } else {
    initAccessPointMode();
  }

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
  globalConfig = newConfig;
  saveConfig();
}

void ProcessingLayer::saveConfig() {
  preferences.begin("device_conf", false);
  preferences.putString("uid", globalConfig.device_uid);
  preferences.putString("wifi_ssid", globalConfig.wifi_ssid);
  preferences.putString("wifi_pass", globalConfig.wifi_pass);
  preferences.putString("mqtt_server", globalConfig.mqtt_server);
  preferences.putInt("mqtt_port", globalConfig.mqtt_port);
  preferences.putString("mqtt_user", globalConfig.mqtt_user);
  preferences.putString("mqtt_pass", globalConfig.mqtt_pass);
  preferences.putString("key_url", globalConfig.key_exchange_url);
  preferences.end();
  ESP_LOGI(TAG, "Configuration saved to flash.");
}

void ProcessingLayer::loadConfig() {
  preferences.begin("device_conf", false);
  globalConfig.device_uid = preferences.getString(
      "uid", ACCESS_TOKEN); // Use ACCESS_TOKEN as default UID
  globalConfig.wifi_ssid = preferences.getString("wifi_ssid", WIFI_SSID);
  globalConfig.wifi_pass = preferences.getString("wifi_pass", WIFI_PASSWORD);
  globalConfig.mqtt_server =
      preferences.getString("mqtt_server", THINGSBOARD_SERVER);
  globalConfig.mqtt_port = preferences.getInt("mqtt_port", 1883);
  globalConfig.mqtt_user = preferences.getString("mqtt_user", "");
  globalConfig.mqtt_pass = preferences.getString("mqtt_pass", "");
  globalConfig.key_exchange_url =
      preferences.getString("key_url", "app.coreiot.io");
  preferences.end();
  ESP_LOGI(TAG, "Configuration loaded from flash. SSID: %s",
           globalConfig.wifi_ssid.c_str());
}
