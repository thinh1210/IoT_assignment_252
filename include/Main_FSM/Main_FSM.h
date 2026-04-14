#ifndef PROCESSING_LAYER_H
#define PROCESSING_LAYER_H

#include "Common/ConfigData.h"
#include "Common/Events.h"
#include "Input/InputLayer.h"
#include "config.h"
#include <Arduino.h>
#include <Preferences.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

class Main_FSM {

private:
  static QueueHandle_t *qInput;
  // State stored in Processing Layer (Rule: No state in WifiService)
  // Task handles for modes
  static TaskHandle_t task_manager_handle;
  static TaskHandle_t task_normal_mode_handle;
  static TaskHandle_t task_manual_mode_handle;
  static TaskHandle_t task_accesspoint_mode_handle;
  static TaskHandle_t task_plant_care_handle;
  static SystemMode currentMode;
  static Preferences preferences;

  static void startManager();
  /**
   *Because there are a little of events in this project, We handleEvent in
   * mainFSM. if many events are in project, we should make handle event for
   * each mode to optimize.
   */
  static void handleEvent(SystemEvent event);
  static void task_plant_care(void *param);

public:
  static void init(QueueHandle_t *qIn);

  // Manager Loop (Always running, receives events)
  static void task_manager(void *param);

  // Worker Loops
  static void task_normal_mode(void *param);
  static void task_manual_mode(void *param);
  static void task_accesspoint_mode(void *param);

  // Mode-specific initialization (Create worker tasks)
  static void initNormalMode();
  static void initManualMode();
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
