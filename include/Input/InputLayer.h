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
    static QueueHandle_t* qProcessing;
    static ButtonHandler* btnHandler;
    static SemaphoreHandle_t btnSemaphore;
    static SystemMode currentMode;

    // Các thẻ quản lý // Task handles for modes
    static TaskHandle_t task_normal_mode_handle;
    static TaskHandle_t task_accesspoint_mode_handle;
    
    static void readSensors();
    static void handleButtonEvents();

    // Worker Loops
    static void task_normal_mode(void* param);
    static void task_accesspoint_mode(void* param);

public:
    static void init(QueueHandle_t* qProc);

    // Normal Mode
    static void initNormalMode();
    static void taskNormalModeLoop(void* param);

    // AccessPoint Mode
    static void initAccessPointMode();
    static void taskAccessPointModeLoop(void* param);

    static void switchMode(SystemMode newMode);
};

#endif // INPUT_LAYER_H
