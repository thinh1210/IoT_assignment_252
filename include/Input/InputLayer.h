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

    // Các thẻ quản lý sub-task cho Mode
    static TaskHandle_t modeATaskHandle;
    static TaskHandle_t modeBTaskHandle;
    
    static void readSensors();
    static void handleButtonEvents();

public:
    static void init(QueueHandle_t* qProc);

    // Mode A
    static void initModeA();
    static void taskModeALoop(void* param);

    // Mode B
    static void initModeB();
    static void taskModeBLoop(void* param);

    static void switchMode(SystemMode newMode);
};

#endif // INPUT_LAYER_H
