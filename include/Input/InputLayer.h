#ifndef INPUT_LAYER_H
#define INPUT_LAYER_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include "config.h"
#include "drivers/ButtonHandler.h"
#include "Common/Events.h"

class InputLayer {
private:
    static QueueHandle_t *qProcessing;
    static ButtonHandler *btnHandler;
    static SemaphoreHandle_t btnSemaphore;
    static SemaphoreHandle_t sensorSemaphore;
    static SystemMode currentMode;

    // Task handles for continuous polling
    static TaskHandle_t task_manager_handle; 
    static TaskHandle_t task_button_handle; 
    static TaskHandle_t task_sensor_handle; 

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
