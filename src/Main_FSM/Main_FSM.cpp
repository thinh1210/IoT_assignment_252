#include "Main_FSM/Main_FSM.h"
#include "drivers/DHTSensor.h"
#include "drivers/LedController.h"
#include "drivers/NeonController.h"
#include "services/DisplayService.h"
#include "services/ManualControlService.h"
#include "services/WifiService.h"
#include "TaskManager.h"
#include "Main_FSM/modes/NormalMode.h"
#include "Main_FSM/modes/ManualMode.h"
#include "Main_FSM/modes/AccessPointMode.h"
#include "Common/AppLog.h"
#include "services/PlantCareRuntimeService.h"

static const char *TAG = "Main_FSM";

namespace {

const char *modeToString(SystemMode mode) {
  switch (mode) {
  case SystemMode::NORMAL_MODE:
    return "NORMAL";
  case SystemMode::ACCESSPOINT_MODE:
    return "ACCESSPOINT";
  case SystemMode::MANUAL_MODE:
    return "MANUAL";
  default:
    return "UNKNOWN";
  }
}

} // namespace

// Define static variables
QueueHandle_t *Main_FSM::qInput = NULL;
TaskHandle_t Main_FSM::task_manager_handle = NULL;
TaskHandle_t Main_FSM::task_normal_mode_handle = NULL;
TaskHandle_t Main_FSM::task_manual_mode_handle = NULL;
TaskHandle_t Main_FSM::task_accesspoint_mode_handle = NULL;
SystemMode Main_FSM::currentMode = SystemMode::NORMAL_MODE;
Preferences Main_FSM::preferences;

/**
 * @brief Initialize the Processing Layer
 */
void Main_FSM::init(QueueHandle_t *qIn) {
  ESP_LOGI(TAG, "General Main_FSM Initialization...");
  qInput = qIn;

  LedController::init();
  NeonController::init();
  ManualControlService::init();
  loadConfig(); // Rule: configuration belongs to processing layer

  // Initialize WiFi logic
  WifiService::init();

  // Rule: State stored here
  currentMode = SystemMode::NORMAL_MODE;

  // Start the persistent Manager Task (Event Handler)
  Main_FSM::startManager();

  ESP_LOGI(TAG, "Processing Layer General Init ready.");
}

void Main_FSM::startManager() {
  xTaskCreate(task_manager, "system_manager", 8192, NULL, 5, &task_manager_handle);
}

void Main_FSM::initNormalMode() {
  deinitMode();
  ESP_LOGI(TAG, "Initializing Processing Normal Mode Worker...");
  xTaskCreate(task_normal_mode, "proc_normal_task", 8192, NULL, 4, &task_normal_mode_handle);
}

void Main_FSM::initManualMode() {
  deinitMode();
  ESP_LOGI(TAG, "Initializing Processing Manual Mode Worker...");
  xTaskCreate(task_manual_mode, "proc_manual_task", 8192, NULL, 4,
              &task_manual_mode_handle);
}

void Main_FSM::initAccessPointMode() {
  deinitMode();
  ESP_LOGI(TAG, "Initializing Processing AccessPoint Mode Worker...");
  xTaskCreate(task_accesspoint_mode, "proc_ap_task", 8192, NULL, 4, &task_accesspoint_mode_handle);
}

void Main_FSM::deinitMode() {
  if (task_normal_mode_handle != NULL) {
    vTaskDelete(task_normal_mode_handle);
    task_normal_mode_handle = NULL;
  }
  if (task_manual_mode_handle != NULL) {
    vTaskDelete(task_manual_mode_handle);
    task_manual_mode_handle = NULL;
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
  if (currentMode == newMode) return;

  ESP_LOGI(TAG, "Mode Transition: %s -> %s", modeToString(currentMode),
           modeToString(newMode));

  if (currentMode == SystemMode::MANUAL_MODE &&
      newMode != SystemMode::MANUAL_MODE) {
    ManualControlService::exitManualMode();
  }

  // 1. Physical/Network transition
  if (newMode == SystemMode::ACCESSPOINT_MODE) {
    WifiService::startAccessPoint();
    DisplayService::showAPMode(1);
  } else if (newMode == SystemMode::MANUAL_MODE) {
    WifiService::startClient();
    ManualControlService::enterManualMode();
    DisplayService::showManualMode();
  } else {
    WifiService::startClient();
    DisplayService::showNormalMode(globalTemp, globalHumi);
  }

  // 2. Delegate Input Layer mode switch (which handles its own task deletion)
  InputLayer::switchMode(newMode);

  // 3. Update own worker tasks
  if (newMode == SystemMode::NORMAL_MODE) {
    initNormalMode();
  } else if (newMode == SystemMode::MANUAL_MODE) {
    initManualMode();
  } else {
    initAccessPointMode();
  }

  currentMode = newMode;
  PlantCareRuntimeService::syncAutomationForMode(newMode);
}

void Main_FSM::handleEvent(SystemEvent event) {
  switch (event.type) {
  case EventType::BUTTON_PRESSED:
    ESP_LOGI(TAG, "Event: Button Pressed. doing some things");
    break;

  case EventType::MODE_CHANGE: {
    SystemMode nextMode = SystemMode::NORMAL_MODE;
    if (currentMode == SystemMode::MANUAL_MODE) {
      nextMode = SystemMode::NORMAL_MODE;
    } else {
      nextMode = (currentMode == SystemMode::NORMAL_MODE)
                     ? SystemMode::ACCESSPOINT_MODE
                     : SystemMode::NORMAL_MODE;
    }
    Main_FSM::switchMode(nextMode);
    break;
  }

  case EventType::SENSOR_DATA_READY:
    NeonController::updateEnvironment(globalTemp, globalHumi);
    break;

  case EventType::IR_REMOTE_CMD:
    if (event.remote_command == RemoteCommand::MODE_NEXT) {
      SystemMode nextMode = SystemMode::NORMAL_MODE;
      switch (currentMode) {
      case SystemMode::NORMAL_MODE:
        nextMode = SystemMode::ACCESSPOINT_MODE;
        break;
      case SystemMode::ACCESSPOINT_MODE:
        nextMode = SystemMode::MANUAL_MODE;
        break;
      case SystemMode::MANUAL_MODE:
      default:
        nextMode = SystemMode::NORMAL_MODE;
        break;
      }
      ESP_LOGI(TAG, "IR mode cycle requested. raw=0x%llX next=%s",
               static_cast<unsigned long long>(event.remote_raw_code),
               modeToString(nextMode));
      Main_FSM::switchMode(nextMode);
    } else if (currentMode == SystemMode::MANUAL_MODE) {
      ESP_LOGI(TAG, "IR manual command received. raw=0x%llX cmd=%d",
               static_cast<unsigned long long>(event.remote_raw_code),
               static_cast<int>(event.remote_command));
      ManualControlService::handleRemoteCommand(event.remote_command);
    } else {
      ESP_LOGW(TAG, "Ignoring IR actuator command outside MANUAL mode. raw=0x%llX",
               static_cast<unsigned long long>(event.remote_raw_code));
    }
    break;

  // --- WebSocket: User saved WiFi/MQTT settings → switch to WiFi mode ---
  case EventType::WS_SAVE_SETTINGS: {
    ConfigData conf = getConfig();
    conf.wifi_ssid   = event.ws_ssid;
    conf.wifi_pass   = event.ws_pass;
    conf.mqtt_user   = event.ws_token;
    conf.mqtt_server = event.ws_server;
    conf.mqtt_port   = event.ws_port;
    updateConfig(conf);
    ESP_LOGI(TAG, "WS_SAVE_SETTINGS: Config saved, switching to NORMAL mode. SSID: %s",
             conf.wifi_ssid.c_str());
    Main_FSM::switchMode(SystemMode::NORMAL_MODE);
    break;
  }

  // --- WebSocket: User toggled/added/deleted a relay ---
  case EventType::WS_RELAY_CMD:
    ESP_LOGI(TAG, "WS_RELAY_CMD: action=%d gpio=%d",
             (int)event.relay_action, event.relay_gpio);
    // GPIO is already handled by ApService directly.
    // This event is available for FSM to react if needed (e.g. log, LED feedback).
    break;

  default:
    break;
  }
}

// --- SUB-TASKS FOR NORMAL MODE ---

void Main_FSM::task_normal_mode(void *param) {
  NormalMode::run(param);
}

void Main_FSM::task_manual_mode(void *param) {
  ManualMode::run(param);
}

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
