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

  static void connectToWifi();
  static void connectToThingsBoard();
};

#endif // CLIENT_SERVICE_H
