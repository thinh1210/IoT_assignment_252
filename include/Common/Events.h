#ifndef EVENTS_H
#define EVENTS_H

#include <Arduino.h>

// Define system modes
enum class SystemMode { NORMAL_MODE, ACCESSPOINT_MODE };

// Event types for communication between Input and Processing
enum class EventType {
  NONE,
  SENSOR_DATA_READY,
  BUTTON_PRESSED,
  SYSTEM_ALERT,
  MODE_CHANGE,
  WS_SAVE_SETTINGS,   // User submitted config form in browser (AP mode)
  WS_RELAY_CMD,       // User toggled/added/deleted relay in browser
};

// Relay command sub-types
enum class RelayAction { ADD, TOGGLE, DELETE };

// Struct to hold event
struct SystemEvent {
  EventType type;
  SystemMode newMode;       // For MODE_CHANGE

  // WS_SAVE_SETTINGS payload
  String ws_ssid;
  String ws_pass;
  String ws_token;
  String ws_server;
  int    ws_port;

  // WS_RELAY_CMD payload
  RelayAction relay_action;
  int relay_gpio;
  String relay_name;
};

#endif // EVENTS_H
