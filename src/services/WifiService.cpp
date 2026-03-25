#include "services/WifiService.h"
#include "services/ApService.h"
#include "services/ClientService.h"
#include "Main_FSM/Main_FSM.h"
#include "esp_log.h"

static const char *TAG = "WifiService";

// Shared web server and WebSocket (lifecycle managed here)
AsyncWebServer WifiService::server(80);
AsyncWebSocket WifiService::ws("/ws");

void WifiService::init() {
  ESP_LOGI(TAG, "Initializing WifiService facade...");

  // Initialize AP service with shared server & WS
  ApService::init(&server, &ws);

  // Start in the correct mode
  if (Main_FSM::getMode() == SystemMode::ACCESSPOINT_MODE) {
    startAccessPoint();
  } else {
    startClient();
  }

  server.begin();
  ESP_LOGI(TAG, "Server started.");
}

void WifiService::startAccessPoint() {
  ApService::startAP();
}

void WifiService::startClient() {
  ClientService::startClient();
}

void WifiService::toggleMode() {
  if (Main_FSM::getMode() == SystemMode::NORMAL_MODE) {
    Main_FSM::switchMode(SystemMode::ACCESSPOINT_MODE);
  } else {
    Main_FSM::switchMode(SystemMode::NORMAL_MODE);
  }
}

void WifiService::maintainConnections() {
  ClientService::maintainConnections();
}

void WifiService::sendTelemetry(float temp, float humi) {
  ClientService::sendTelemetry(temp, humi);
}

void WifiService::broadcastTelemetry(float temp, float humi) {
  ApService::broadcastTelemetry(temp, humi);
}

void WifiService::loop() {}
