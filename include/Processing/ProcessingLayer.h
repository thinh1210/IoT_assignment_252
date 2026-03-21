#ifndef PROCESSING_LAYER_H
#define PROCESSING_LAYER_H

#include "Common/Events.h"
#include "Input/InputLayer.h"
#include "config.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

class ProcessingLayer {
private:
  static QueueHandle_t *qInput;
  static SystemMode currentMode;

  // Task handles for modes
  static TaskHandle_t modeATaskHandle;
  static TaskHandle_t modeBTaskHandle;

  // Handle events from queue
  static void handleEvent(SystemEvent event);

public:
  static void init(QueueHandle_t *qIn);

  // Mode A
  static void initModeA();
  static void taskModeALoop(void *param);

  // Mode B
  static void initModeB();
  static void taskModeBLoop(void *param);

  // Orchestration API
  static void switchMode(SystemMode newMode);
};

#endif // PROCESSING_LAYER_H
