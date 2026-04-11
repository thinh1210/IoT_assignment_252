#include "services/ClientService.h"
#include "Main_FSM/Main_FSM.h"
#include "services/CoreIotPublishService.h"
#include "config.h"
#include "esp_log.h"

#if THINGSBOARD_ENABLE_OTA
#include <Arduino_ESP32_Updater.h>
#include <OTA_Update_Callback.h>
#endif

static const char *CLI_TAG = "ClientService";

namespace {

String resolveAccessToken(const ConfigData &conf) {
  if (!conf.mqtt_user.isEmpty()) {
    return conf.mqtt_user;
  }
  if (!conf.device_uid.isEmpty()) {
    return conf.device_uid;
  }
  return String(ACCESS_TOKEN);
}

String resolveServer(const ConfigData &conf) {
  if (!conf.mqtt_server.isEmpty()) {
    return conf.mqtt_server;
  }
  return String(THINGSBOARD_SERVER);
}

uint16_t resolvePort(const ConfigData &conf) {
  if (conf.mqtt_port > 0) {
    return static_cast<uint16_t>(conf.mqtt_port);
  }
  return THINGSBOARD_PORT;
}

#if THINGSBOARD_ENABLE_OTA

constexpr uint32_t kOtaRestartDelayMs = 1500U;
constexpr char kOtaUpdatedState[] = "UPDATED";

Arduino_ESP32_Updater gOtaUpdater;
bool gOtaSubscriptionReady = false;
bool gOtaRestartPending = false;
bool gOtaUpdatedStateSent = false;
uint32_t gOtaRestartRequestedAt = 0U;

void handleOtaProgress(const size_t &current, const size_t &total) {
  ESP_LOGI(CLI_TAG, "OTA progress: %u/%u", static_cast<unsigned>(current),
           static_cast<unsigned>(total));
}

void handleOtaFinished(const bool &success) {
  if (!success) {
    ESP_LOGE(CLI_TAG, "OTA update failed.");
    gOtaRestartPending = false;
    gOtaUpdatedStateSent = false;
    return;
  }

  ESP_LOGI(CLI_TAG, "OTA image applied. Scheduling reboot...");
  gOtaRestartPending = true;
  gOtaUpdatedStateSent = false;
  gOtaRestartRequestedAt = millis();
}

OTA_Update_Callback &getOtaCallback() {
  static OTA_Update_Callback callback(handleOtaProgress, handleOtaFinished,
                                      COREIOT_FW_TITLE, COREIOT_FW_VERSION,
                                      &gOtaUpdater);
  return callback;
}

bool setupOta(ThingsBoard &tb) {
  OTA_Update_Callback &callback = getOtaCallback();
  bool ok = true;

  if (!gOtaSubscriptionReady) {
    gOtaSubscriptionReady = tb.Subscribe_Firmware_Update(callback);
    if (!gOtaSubscriptionReady) {
      ESP_LOGE(CLI_TAG, "Failed to subscribe for OTA update notifications.");
      ok = false;
    }
  }

  if (!tb.Start_Firmware_Update(callback)) {
    ESP_LOGE(CLI_TAG, "Failed to request current OTA firmware metadata.");
    ok = false;
  }

  return ok;
}

void processPendingOtaRestart(ThingsBoard &tb) {
  if (!gOtaRestartPending) {
    return;
  }

  if (!gOtaUpdatedStateSent && tb.connected()) {
    if (tb.Firmware_Send_State(kOtaUpdatedState)) {
      gOtaUpdatedStateSent = true;
      ESP_LOGI(CLI_TAG, "OTA state UPDATED sent to CoreIoT.");
    }
  }

  if (millis() - gOtaRestartRequestedAt >= kOtaRestartDelayMs) {
    ESP_LOGI(CLI_TAG, "Restarting device after OTA update.");
    ESP.restart();
  }
}

#endif // THINGSBOARD_ENABLE_OTA

} // namespace

// === Static member definitions ===
WiFiClient ClientService::espClient;
Arduino_MQTT_Client ClientService::tbClient(ClientService::espClient);
ThingsBoard ClientService::tb(ClientService::tbClient);

// ============================================================
//  START CLIENT MODE
// ============================================================

void ClientService::startClient() {
  ESP_LOGI(CLI_TAG, "Starting WiFi Client mode...");
  WiFi.mode(WIFI_OFF);
  vTaskDelay(pdMS_TO_TICKS(100));
  WiFi.mode(WIFI_STA);
  connectToWifi();
}

void ClientService::connectToWifi() {
  if (WiFi.status() == WL_CONNECTED) return;
  ConfigData &conf = Main_FSM::getConfig();
  ESP_LOGI(CLI_TAG, "Connecting to WiFi: %s", conf.wifi_ssid.c_str());
  WiFi.begin(conf.wifi_ssid.c_str(), conf.wifi_pass.c_str());
}

void ClientService::connectToThingsBoard() {
  if (tb.connected()) return;
  ConfigData &conf = Main_FSM::getConfig();
  const String server = resolveServer(conf);
  const String token = resolveAccessToken(conf);
  const uint16_t port = resolvePort(conf);

  ESP_LOGI(CLI_TAG, "Connecting to CoreIoT %s:%u", server.c_str(), port);
  if (!tb.connect(server.c_str(), token.c_str(), port)) {
    ESP_LOGE(CLI_TAG, "CoreIoT connect FAILED!");
  } else {
    ESP_LOGI(CLI_TAG, "CoreIoT connected.");
#if THINGSBOARD_ENABLE_OTA
    if (!setupOta(tb)) {
      ESP_LOGW(CLI_TAG, "CoreIoT OTA setup incomplete.");
    }
#endif
  }
}

// ============================================================
//  MAINTAIN CONNECTIONS (called from NormalMode task loop)
// ============================================================

void ClientService::maintainConnections() {
  uint32_t now = millis();
  static uint32_t lastWifiRetry = 0;
  static uint32_t lastTBRetry   = 0;

#if THINGSBOARD_ENABLE_OTA
  processPendingOtaRestart(tb);
#endif

  if (WiFi.status() != WL_CONNECTED) {
    if (now - lastWifiRetry >= 15000) {
      connectToWifi();
      lastWifiRetry = now;
    }
  } else {
    lastWifiRetry = now;
    if (!tb.connected()) {
      if (now - lastTBRetry >= 10000) {
        connectToThingsBoard();
        lastTBRetry = now;
      }
    } else {
      lastTBRetry = now;
      tb.loop();
    }
  }
}

// ============================================================
//  TELEMETRY
// ============================================================

void ClientService::sendTelemetry(float temp, float humi) {
  if (!tb.connected()) return;
  const CoreIotTelemetrySnapshot snapshot =
      CoreIotPublishService::buildSnapshot(temp, humi);
  const bool published = CoreIotPublishService::publish(tb, snapshot);

  if (!published) {
    ESP_LOGW(CLI_TAG, "CoreIoT telemetry publish incomplete.");
  }

  ESP_LOGD(CLI_TAG,
           "Telemetry sent -> T=%.1f H=%.0f alert=%d fan=%d pump=%d ml=%s",
           snapshot.temperature, snapshot.humidity, snapshot.alertStatus,
           snapshot.fanState, snapshot.pumpState, snapshot.tinyMlResult.c_str());
}

bool ClientService::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}
