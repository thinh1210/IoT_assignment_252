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
  static Preferences preferences;

  // Task handles for modes
  static TaskHandle_t task_manager_handle;
  static TaskHandle_t task_normal_mode_handle;
  static TaskHandle_t task_accesspoint_mode_handle;

  static void startManager();
  static void handleEvent(SystemEvent event);

public:
  static void init(QueueHandle_t *qIn);

  // Manager Loop (Always running, receives events)
  static void task_manager(void *param);

  // Worker Loops
  static void task_normal_mode(void *param);
  static void task_accesspoint_mode(void *param);

  // Mode-specific initialization (Create worker tasks)
  static void initNormalMode();
  static void initAccessPointMode();

  // Cleanup current mode tasks
  static void deinitMode();

  // Orchestration API
  static void switchMode(SystemMode newMode);
  static SystemMode getMode() { return currentMode; }

  // State Access
  static String getDeviceUID() { return globalConfig.device_uid; }
  static ConfigData &getConfig() { return globalConfig; }
  static void updateConfig(const ConfigData &newConfig);
  static void saveConfig();
  static void loadConfig();
};

#endif // PROCESSING_LAYER_H
