#ifndef AP_SERVICE_H
#define AP_SERVICE_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <freertos/semphr.h>
#include <vector>
#include "Common/Events.h"

/**
 * @brief ApService - Manages the ESP32 Access Point mode.
 * Responsibilities:
 *  - Start/stop WiFi AP
 *  - Serve the Web Dashboard (HTML, CSS, JS)
 *  - Manage WebSocket connections
 *  - Stream DHT sensor data to browser periodically
 *  - Parse WS commands and push them as SystemEvents into InputLayer's queue
 *  - Persist relay list in NVS across mode switches
 */

struct RelayConfig {
  String name;
  int gpio;
  bool state;
};

class ApService {
public:
  static void init(AsyncWebServer *sharedServer, AsyncWebSocket *sharedWs);
  static void startAP();
  static void stopAP();
  static void broadcastTelemetry(float temp, float humi);
  static void broadcastRelayList();
  static void loadRelays();
  static void saveRelays();
  static bool setRelayState(int gpio, bool state, bool persistState = true);

  // Relay access (needed by ClientService for GPIO state restore)
  static std::vector<RelayConfig>& getRelayList();
  static SemaphoreHandle_t getRelayMutex();

private:
  static AsyncWebServer *server;
  static AsyncWebSocket  *ws;

  static std::vector<RelayConfig> relayList;
  static SemaphoreHandle_t relayMutex;

  static void onWsEvent(AsyncWebSocket *s, AsyncWebSocketClient *c,
                        AwsEventType type, void *arg, uint8_t *data, size_t len);
  static void handleWsMessage(void *arg, uint8_t *data, size_t len);
  static void applyRelayGPIO(const RelayConfig &r);
  static void syncAutomationRelays();
};

#endif // AP_SERVICE_H
