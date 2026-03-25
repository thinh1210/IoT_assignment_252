#include "services/ClientService.h"
#include "Main_FSM/Main_FSM.h"
#include "config.h"
#include "esp_log.h"

static const char *CLI_TAG = "ClientService";

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
  String uid = Main_FSM::getDeviceUID();
  ESP_LOGI(CLI_TAG, "Connecting to ThingsBoard as: %s", uid.c_str());
  if (!tb.connect(THINGSBOARD_SERVER, uid.c_str())) {
    ESP_LOGE(CLI_TAG, "ThingsBoard connect FAILED!");
  } else {
    ESP_LOGI(CLI_TAG, "ThingsBoard connected.");
  }
}

// ============================================================
//  MAINTAIN CONNECTIONS (called from NormalMode task loop)
// ============================================================

void ClientService::maintainConnections() {
  uint32_t now = millis();
  static uint32_t lastWifiRetry = 0;
  static uint32_t lastTBRetry   = 0;

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
  tb.sendTelemetryData("temperature", temp);
  tb.sendTelemetryData("humidity", humi);
  ESP_LOGD(CLI_TAG, "Telemetry sent → T=%.1f H=%.0f", temp, humi);
}

bool ClientService::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}
