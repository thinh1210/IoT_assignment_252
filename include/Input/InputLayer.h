#ifndef INPUT_LAYER_H
#define INPUT_LAYER_H

#include "Common/Events.h"
#include "config.h"
#include "drivers/ButtonHandler.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
// Can remake this layer: Can turn on and turn off the input task -> future work
class InputLayer {
private:
  static QueueHandle_t *qProcessing;
  static ButtonHandler *btnHandler;
  static SystemMode currentMode;
  
  // Global handles are now used directly via config.h

public:
  // General layer init: Setup hardware and persistent tasks
  static void init(QueueHandle_t *qProc);

  // Persistent Input Manager (Rule: handle event task)
  static void task_manager(void *param);

  // Orchestrate transitions (clean if needed)
  static void switchMode(SystemMode newMode);

  // Allow external services (e.g. ApService) to push events into the queue
  static void pushEvent(const SystemEvent &event);
};

#endif // INPUT_LAYER_H
