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
bool ClientService::wifiEventsRegistered = false;

// ============================================================
//  START CLIENT MODE
// ============================================================

void ClientService::startClient() {
  registerWifiEvents();
  ESP_LOGI(CLI_TAG, "Starting WiFi Client mode...");
  WiFi.mode(WIFI_OFF);
  vTaskDelay(pdMS_TO_TICKS(100));
  WiFi.mode(WIFI_STA);
  connectToWifi();
}

void ClientService::connectToWifi() {
  if (WiFi.status() == WL_CONNECTED) return;
  ConfigData &conf = Main_FSM::getConfig();
  ESP_LOGI(CLI_TAG, "Connecting to WiFi: SSID=%s, pass_len=%u, status=%s",
           conf.wifi_ssid.c_str(), conf.wifi_pass.length(),
           wifiStatusToString(WiFi.status()));
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
  ConfigData &conf = Main_FSM::getConfig();
  const char *server = conf.mqtt_server.isEmpty() ? THINGSBOARD_SERVER
                                                  : conf.mqtt_server.c_str();
  const uint16_t port = conf.mqtt_port > 0 ? conf.mqtt_port : 1883;
  String accessToken =
      conf.mqtt_user.isEmpty() ? Main_FSM::getDeviceUID() : conf.mqtt_user;

  ESP_LOGI(CLI_TAG,
           "Connecting to ThingsBoard: host=%s, port=%u, token_len=%u",
           server, port, accessToken.length());
  if (!tb.connect(server, accessToken.c_str(), port)) {
    ESP_LOGE(CLI_TAG, "ThingsBoard connect FAILED! host=%s port=%u", server,
             port);
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
  static uint32_t lastSummary   = 0;
  static wl_status_t lastWifiStatus = WL_NO_SHIELD;
  static bool lastTbConnected = false;

  const wl_status_t wifiStatus = WiFi.status();
  const bool tbConnected = tb.connected();

  if (wifiStatus != lastWifiStatus) {
    ESP_LOGI(CLI_TAG, "WiFi status changed: %s -> %s",
             wifiStatusToString(lastWifiStatus), wifiStatusToString(wifiStatus));
    lastWifiStatus = wifiStatus;
  }

  if (tbConnected != lastTbConnected) {
    ESP_LOGI(CLI_TAG, "ThingsBoard state changed: %s -> %s",
             lastTbConnected ? "CONNECTED" : "DISCONNECTED",
             tbConnected ? "CONNECTED" : "DISCONNECTED");
    lastTbConnected = tbConnected;
  }

  if (wifiStatus != WL_CONNECTED) {
    if (now - lastWifiRetry >= 15000) {
      ESP_LOGW(CLI_TAG, "WiFi not connected. Scheduling reconnect...");
      connectToWifi();
      lastWifiRetry = now;
    }
  } else {
    lastWifiRetry = now;
    if (!tbConnected) {
      if (now - lastTBRetry >= 10000) {
        ESP_LOGW(CLI_TAG, "WiFi ready but ThingsBoard disconnected. Retrying...");
        connectToThingsBoard();
        lastTBRetry = now;
      }
    } else {
      lastTBRetry = now;
      tb.loop();
    }
  }

  if (now - lastSummary >= 10000) {
    ESP_LOGI(CLI_TAG, "Connection summary: WiFi=%s, IP=%s, RSSI=%d, TB=%s",
             wifiStatusToString(wifiStatus), WiFi.localIP().toString().c_str(),
             WiFi.RSSI(), tbConnected ? "CONNECTED" : "DISCONNECTED");
    lastSummary = now;
  }
}

// ============================================================
//  TELEMETRY
// ============================================================

void ClientService::sendTelemetry(float temp, float humi) {
  if (!tb.connected()) return;
  tb.sendTelemetryData("temperature", temp);
  tb.sendTelemetryData("humidity", humi);
  ESP_LOGI(CLI_TAG, "Telemetry sent -> T=%.1f C, H=%.0f %%", temp, humi);
}

bool ClientService::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void ClientService::registerWifiEvents() {
  if (wifiEventsRegistered) return;

  WiFi.onEvent(onWifiEvent);
  wifiEventsRegistered = true;
  ESP_LOGI(CLI_TAG, "WiFi event logger registered.");
}

void ClientService::onWifiEvent(arduino_event_id_t event,
                                arduino_event_info_t info) {
  switch (event) {
  case ARDUINO_EVENT_WIFI_STA_START:
    ESP_LOGI(CLI_TAG, "[WiFi Event] STA started.");
    break;
  case ARDUINO_EVENT_WIFI_STA_CONNECTED:
    ESP_LOGI(CLI_TAG, "[WiFi Event] Connected to AP, waiting for IP...");
    break;
  case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    ESP_LOGI(CLI_TAG, "[WiFi Event] Got IP: %s",
             IPAddress(info.got_ip.ip_info.ip.addr).toString().c_str());
    break;
  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    ESP_LOGW(CLI_TAG, "[WiFi Event] Disconnected. reason=%u (%s)",
             info.wifi_sta_disconnected.reason,
             WiFi.disconnectReasonName(
                 (wifi_err_reason_t)info.wifi_sta_disconnected.reason));
    break;
  case ARDUINO_EVENT_WIFI_STA_LOST_IP:
    ESP_LOGW(CLI_TAG, "[WiFi Event] Lost IP address.");
    break;
  default:
    break;
  }
}

const char *ClientService::wifiStatusToString(wl_status_t status) {
  switch (status) {
  case WL_NO_SHIELD:
    return "NO_SHIELD";
  case WL_IDLE_STATUS:
    return "IDLE";
  case WL_NO_SSID_AVAIL:
    return "NO_SSID";
  case WL_SCAN_COMPLETED:
    return "SCAN_DONE";
  case WL_CONNECTED:
    return "CONNECTED";
  case WL_CONNECT_FAILED:
    return "CONNECT_FAILED";
  case WL_CONNECTION_LOST:
    return "CONNECTION_LOST";
  case WL_DISCONNECTED:
    return "DISCONNECTED";
  default:
    return "UNKNOWN";
  }
}
