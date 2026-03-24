#include "Processing/WifiModeManager.h"
#include "Processing/ProcessingLayer.h"
#include "data/WebPortal.h"
#include "config.h"
#include "esp_log.h"

static const char *TAG = "WifiModeMan";

// Static server initialization
AsyncWebServer WifiModeManager::server(80);

// WiFi & ThingsBoard client instantiation
WiFiClient WifiModeManager::espClient;
Arduino_MQTT_Client WifiModeManager::tbClient(WifiModeManager::espClient);
ThingsBoard WifiModeManager::tb(WifiModeManager::tbClient);

void WifiModeManager::init() {
  ESP_LOGI(TAG, "Initializing WiFi & Web Config Manager...");

  // Setup Routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", index_html);
  });

  server.on(
      "/config", HTTP_POST,
      [](AsyncWebServerRequest *request) {
        request->send(200, "application/json", "{\"status\":\"ok\"}");
      },
      NULL, handleConfigData);

  server.on("/toggle", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Toggling...");
    toggleMode();
  });

  // Start initial mode based on ProcessingLayer state
  if (ProcessingLayer::getMode() == SystemMode::ACCESSPOINT_MODE) {
    startAccessPoint();
  } else {
    startClient();
  }
     
  server.begin();
  ESP_LOGI(TAG, "Web Server started.");
}

void WifiModeManager::toggleMode() {
  if (ProcessingLayer::getMode() == SystemMode::NORMAL_MODE) {
    ESP_LOGI(TAG, "User requested ACCESSPOINT mode.");
    ProcessingLayer::switchMode(SystemMode::ACCESSPOINT_MODE);
  } else {
    ESP_LOGI(TAG, "User requested NORMAL mode.");
    ProcessingLayer::switchMode(SystemMode::NORMAL_MODE);
  }
}

void WifiModeManager::loop() {}

bool WifiModeManager::isAccessPoint() {
  return (ProcessingLayer::getMode() == SystemMode::ACCESSPOINT_MODE);
}

void WifiModeManager::startClient() {
  ESP_LOGI(TAG, "Starting CLIENT mode...");
  
  WiFi.mode(WIFI_OFF);
  vTaskDelay(pdMS_TO_TICKS(100)); // Safety delay using vTaskDelay
  
  WiFi.mode(WIFI_STA);
  ConfigData &conf = ProcessingLayer::getConfig();
  WiFi.begin(conf.wifi_ssid.c_str(), conf.wifi_pass.c_str());
  
  ESP_LOGI(TAG, "Connecting to %s...", conf.wifi_ssid.c_str());
}

void WifiModeManager::startAccessPoint() {
  ESP_LOGI(TAG, "Starting ACCESSPOINT mode...");

  // 1. Cleanup existing services
  if (tb.connected()) tb.disconnect();
  
  WiFi.mode(WIFI_OFF);
  vTaskDelay(pdMS_TO_TICKS(100)); // Safety delay

  // 2. Configure AP Mode
  WiFi.mode(WIFI_AP);

  IPAddress local_IP(ACCESSPOINT_IP);
  IPAddress gateway(ACCESSPOINT_GATEWAY);
  IPAddress subnet(ACCESSPOINT_SUBNET);
  WiFi.softAPConfig(local_IP, gateway, subnet);

  // 3. Start AP
  if (WiFi.softAP(ACCESSPOINT_SSID, ACCESSPOINT_PASS)) {
    ESP_LOGI(TAG, "AP Started. SSID: %s, IP: %s", ACCESSPOINT_SSID,
             WiFi.softAPIP().toString().c_str());
  } else {
    ESP_LOGE(TAG, "AccessPoint Failed!");
  }
}

void WifiModeManager::handleConfigData(AsyncWebServerRequest *request,
                                       uint8_t *data, size_t len, size_t index,
                                       size_t total) {
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, data, len);

  if (error) {
    ESP_LOGE(TAG, "JSON error");
    return;
  }

  ConfigData &conf = ProcessingLayer::getConfig();
  if (doc.containsKey("wifi_ssid")) conf.wifi_ssid = doc["wifi_ssid"].as<String>();
  if (doc.containsKey("wifi_pass")) conf.wifi_pass = doc["wifi_pass"].as<String>();
  if (doc.containsKey("mqtt_server")) conf.mqtt_server = doc["mqtt_server"].as<String>();
  if (doc.containsKey("mqtt_port")) conf.mqtt_port = doc["mqtt_port"] | 1883;
  if (doc.containsKey("mqtt_user")) conf.mqtt_user = doc["mqtt_user"].as<String>();
  if (doc.containsKey("key_url")) conf.key_exchange_url = doc["key_url"].as<String>();

  ProcessingLayer::saveConfig();
  ESP_LOGI(TAG, "Config updated. WiFi SSID: %s", conf.wifi_ssid.c_str());
}

void WifiModeManager::maintainConnections() {
  uint32_t now = millis();
  static uint32_t lastWifiRetry = 0;
  static uint32_t lastTBRetry = 0;

  if (ProcessingLayer::getMode() != SystemMode::ACCESSPOINT_MODE) {
    // Check WiFi
    if (WiFi.status() != WL_CONNECTED) {
      if (now - lastWifiRetry >= 15000) {
        connectToWiFiInternal();
        lastWifiRetry = now;
      }
    } else {
      lastWifiRetry = now;
      // Check ThingsBoard only if WiFi is connected
      if (!tb.connected()) {
        if (now - lastTBRetry >= 10000) {
          connectToThingsBoardInternal();
          lastTBRetry = now;
        }
      } else {
        lastTBRetry = now;
        tb.loop();
      }
    }
  }
}

void WifiModeManager::sendTelemetry(float temp, float humi) {
  if (tb.connected()) {
    tb.sendTelemetryData("temperature", temp);
    tb.sendTelemetryData("humidity", humi);
  }
}

void WifiModeManager::connectToWiFiInternal() {
  if (WiFi.status() == WL_CONNECTED) return;
  ConfigData &conf = ProcessingLayer::getConfig();
  WiFi.begin(conf.wifi_ssid.c_str(), conf.wifi_pass.c_str());
}

void WifiModeManager::connectToThingsBoardInternal() {
  if (tb.connected()) return;
  String deviceUID = ProcessingLayer::getDeviceUID();
  if (!tb.connect(THINGSBOARD_SERVER, deviceUID.c_str())) {
    ESP_LOGE(TAG, "TB Connect Failed!");
  } else {
    ESP_LOGI(TAG, "TB Connected.");
  }
}
