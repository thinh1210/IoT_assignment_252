#include "Main_FSM/Main_FSM.h"
#include "Main_FSM/modes/AccessPointMode.h"
#include "Main_FSM/modes/NormalMode.h"
#include "TaskManager.h"
#include "config.h"
#include "drivers/DHTSensor.h"
#include "drivers/LedController.h"
#include "drivers/NeonController.h"
#include "esp_log.h"
#include "services/DisplayService.h"
#include "services/WifiService.h"

static const char *TAG = "Main_FSM";

// Define static variables
QueueHandle_t *Main_FSM::qInput = NULL;
SystemMode Main_FSM::currentMode = static_cast<SystemMode>(-1); // Uninitialized
Preferences Main_FSM::preferences;

/**
 * @brief Initialize the Processing Layer
 */
void Main_FSM::init(QueueHandle_t *qIn) {
  ESP_LOGI(TAG, "General Main_FSM Initialization...");
  qInput = qIn;

  LedController::init();
  NeonController::init();
  loadConfig(); // Rule: configuration belongs to processing layer

  // Initialize WiFi logic
  WifiService::init();

  // Start the persistent Manager Task (Event Handler)
  Main_FSM::startManager();

  ESP_LOGI(TAG, "Processing Layer General Init ready.");
}

void Main_FSM::startManager() {
  xTaskCreate(task_manager, "system_manager", 8192, NULL, 5,
              &task_sys_manager_handle);
}

/**
 * @brief Manager Task: Handle Input from queue and manage task states of all
 * layers
 */
void Main_FSM::task_manager(void *param) {
  SystemEvent event;
  ESP_LOGI(TAG, "Manager Task started. Listening for Input events...");

  while (1) {
    // Wait for event from Input Layer (Rule 4.2)
    if (qInput != NULL &&
        xQueueReceive(*qInput, &event, pdMS_TO_TICKS(100)) == pdPASS) {
      Main_FSM::handleEvent(event);
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

/**
 * @brief Orchestrate mode switching by suspending/resuming tasks across layers
 */
void Main_FSM::switchMode(SystemMode newMode) {
  if (currentMode == newMode)
    return;

  ESP_LOGI(TAG, "Mode Transition: %d -> %d", (int)currentMode, (int)newMode);

  // 2. Delegate Input Layer mode switch
  InputLayer::switchMode(newMode);

  // 3. Delegate to Worker Modes to stop old tasks
  switch (currentMode) {
  case SystemMode::NORMAL_MODE:
    NormalMode::exit();
    break;
  case SystemMode::ACCESSPOINT_MODE:
    AccessPointMode::exit();
    break;
  default:
    break;
  }

  // 4. Delegate to Worker Modes to start new tasks
  switch (newMode) {
  case SystemMode::NORMAL_MODE:
    NormalMode::enter();
    break;
  case SystemMode::ACCESSPOINT_MODE:
    AccessPointMode::enter();
    break;
  default:
    break;
  }

  currentMode = newMode;
}

void Main_FSM::handleEvent(SystemEvent event) {
  switch (event.type) {
  case EventType::BUTTON_PRESSED:
    ESP_LOGI(TAG, "Event: Button Pressed. Action: Change Neon color.");
    NeonController::setNextColor();
    break;

  case EventType::MODE_CHANGE: {
    SystemMode nextMode = (currentMode == SystemMode::NORMAL_MODE)
                              ? SystemMode::ACCESSPOINT_MODE
                              : SystemMode::NORMAL_MODE;
    Main_FSM::switchMode(nextMode);
    break;
  }

  case EventType::SENSOR_DATA_READY:
    // Telemetry is handled in the normal worker task loop
    DisplayService::showNormalMode(globalTemp, globalHumi);
    break;

  // --- WebSocket: User saved WiFi/MQTT settings → switch to WiFi mode ---
  case EventType::WS_SAVE_SETTINGS: {
    ConfigData conf = getConfig();
    conf.wifi_ssid = event.ws_ssid;
    conf.wifi_pass = event.ws_pass;
    conf.mqtt_user = event.ws_token;
    conf.mqtt_server = event.ws_server;
    conf.mqtt_port = event.ws_port;
    updateConfig(conf);
    ESP_LOGI(
        TAG,
        "WS_SAVE_SETTINGS: Config saved, switching to NORMAL mode. SSID: %s",
        conf.wifi_ssid.c_str());
    Main_FSM::switchMode(SystemMode::NORMAL_MODE);
    break;
  }

  // --- WebSocket: User toggled/added/deleted a relay ---
  case EventType::WS_RELAY_CMD:
    ESP_LOGI(TAG, "WS_RELAY_CMD: action=%d gpio=%d", (int)event.relay_action,
             event.relay_gpio);
    // GPIO is already handled by ApService directly.
    // This event is available for FSM to react if needed (e.g. log, LED
    // feedback).
    break;

  default:
    break;
  }
}

// --- SUB-TASKS FOR NORMAL MODE ---

void Main_FSM::task_normal_mode(void *param) { NormalMode::run(param); }

// --- SUB-TASKS FOR ACCESSPOINT MODE ---

void Main_FSM::task_accesspoint_mode(void *param) {
  AccessPointMode::run(param);
}

void Main_FSM::updateConfig(const ConfigData &newConfig) {
  globalConfig = newConfig;
  saveConfig();
}

void Main_FSM::saveConfig() {
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

void Main_FSM::loadConfig() {
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
