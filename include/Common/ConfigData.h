#ifndef CONFIG_DATA_H
#define CONFIG_DATA_H

#include <Arduino.h>

struct ConfigData {
  String wifi_ssid;
  String wifi_pass;
  String mqtt_server;
  int mqtt_port;
  String mqtt_user;
  String mqtt_pass;
  String key_exchange_url;
};

#endif // CONFIG_DATA_H
