#ifndef CLIENT_SERVICE_H
#define CLIENT_SERVICE_H

#include <Arduino.h>
#include <WiFi.h>
#include <ThingsBoard.h>
#include <Arduino_MQTT_Client.h>
#include "Common/ConfigData.h"

/**
 * @brief ClientService - Manages WiFi Station mode and ThingsBoard MQTT.
 * Responsibilities:
 *  - Connect / reconnect WiFi
 *  - Connect / reconnect ThingsBoard
 *  - Send telemetry to cloud
 */
class ClientService {
public:
  static void startClient();
  static void maintainConnections();
  static void sendTelemetry(float temp, float humi);

  static bool isConnected();

private:
  static WiFiClient espClient;
  static Arduino_MQTT_Client tbClient;
  static ThingsBoard tb;
  static bool wifiEventsRegistered;

  static void connectToWifi();
  static void connectToThingsBoard();
  static void registerWifiEvents();
  static void onWifiEvent(arduino_event_id_t event, arduino_event_info_t info);
  static const char *wifiStatusToString(wl_status_t status);
};

#endif // CLIENT_SERVICE_H
