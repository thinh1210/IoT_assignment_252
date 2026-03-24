#ifndef INPUT_LAYER_H
#define INPUT_LAYER_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include "config.h"
#include "Input/ButtonHandler.h"
#include "Common/Events.h"

class InputLayer {
private:
    static QueueHandle_t *qProcessing;
    static ButtonHandler *btnHandler;
    static SemaphoreHandle_t btnSemaphore;
    static SystemMode currentMode;

    // Task handles for separate modes
    static TaskHandle_t task_manager_handle; // Persistent
    static TaskHandle_t task_normal_mode_handle; // Mode specific
    static TaskHandle_t task_accesspoint_mode_handle; // Mode specific

    static void handleButtonEvents();
    static void readSensors();

public:
    // General layer init: Setup hardware and persistent tasks
    static void init(QueueHandle_t *qProc);
    
    // Persistent Input Manger (Rule: handle event task)
    static void task_manager(void *param);

    // Mode-specific initialization (Create tasks)
    static void initNormalMode();
    static void initAccessPointMode();

    // Cleanup current mode tasks
    static void deinitMode();

    // Orchestrate transitions
    static void switchMode(SystemMode newMode);

    // Mode specific worker tasks
    static void task_normal_mode(void *param);
    static void task_accesspoint_mode(void *param);
};

#endif // INPUT_LAYER_H
