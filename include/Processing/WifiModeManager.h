#ifndef WIFI_MODE_MANAGER_H
#define WIFI_MODE_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <ThingsBoard.h>
#include <Arduino_MQTT_Client.h>
#include "Common/ConfigData.h"

class WifiModeManager {

public:
    static void init();
    static void toggleMode();
    static void loop();
    static void maintainConnections();
    static void sendTelemetry(float temp, float humi);
    static void startAccessPoint();
    static void startClient();

private:
    static AsyncWebServer server;
    // State removed from here, now in ProcessingLayer

    // WiFi & ThingsBoard client
    static WiFiClient espClient;
    static Arduino_MQTT_Client tbClient;
    static ThingsBoard tb;

    static void handleConfigData(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    static void connectToWiFiInternal();
    static void connectToThingsBoardInternal();

    static bool isAccessPoint(); 
};

#endif // WIFI_MODE_MANAGER_H

