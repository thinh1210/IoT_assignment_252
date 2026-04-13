#include "services/ClientService.h"
#include "Main_FSM/Main_FSM.h"
#include "config.h"
#include "esp_log.h"

static const char *CLI_TAG = "ClientService";

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
    ESP_LOGI(CLI_TAG, "ThingsBoard connected to %s:%u", server, port);
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
