#ifndef WIFI_MODE_MANAGER_H
#define WIFI_MODE_MANAGER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "Common/Events.h"

/**
 * @brief WifiService - Facade that coordinates ApService and ClientService.
 * External code should only use this class for WiFi operations.
 */
class WifiService {
public:
  static void init();
  static void startAccessPoint();
  static void startClient();
  static void toggleMode();
  static void loop();
  static void maintainConnections();
  static void sendTelemetry(float temp, float humi);
  static void broadcastTelemetry(float temp, float humi);

private:
  static AsyncWebServer server;
  static AsyncWebSocket ws;
};

#endif // WIFI_MODE_MANAGER_H
