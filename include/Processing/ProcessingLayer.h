#ifndef PROCESSING_LAYER_H
#define PROCESSING_LAYER_H

#include "Common/Events.h"
#include "Input/InputLayer.h"
#include "config.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <Preferences.h>
#include "Common/ConfigData.h"

class ProcessingLayer {

private:
  static QueueHandle_t *qInput;
  // State stored in Processing Layer (Rule: No state in WifiModeManager)
  static SystemMode currentMode;
  static String deviceUID;
  static ConfigData currentConfig;
  static Preferences preferences;

  // Task handles for modes
  static TaskHandle_t task_manager_handle;
  static TaskHandle_t task_normal_mode_handle;
  static TaskHandle_t task_accesspoint_mode_handle;

  // Internal Logic
  static void handleEvent(SystemEvent event);

public:
  static void init(QueueHandle_t *qIn);

  // Manager Loop (Always running, receives events)
  static void task_manager(void *param);

  // Worker Loops
  static void task_normal_mode(void *param);
  static void task_accesspoint_mode(void *param);

  // Initialization methods
  static void startManager();
  static void initNormalMode();
  static void initAccessPointMode();

  // Orchestration API
  static void switchMode(SystemMode newMode);
  static SystemMode getMode() { return currentMode; }

  // State Access (Rule: WifiModeManager uses these)
  static String getDeviceUID() { return deviceUID; }
  static ConfigData &getConfig() { return currentConfig; }
  static void updateConfig(const ConfigData &newConfig);
  static void saveConfig();
  static void loadConfig();
};

#endif // PROCESSING_LAYER_H
