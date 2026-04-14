#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "config.h"

class LedController {
public:
    static void init();
    static void setState(bool on);
    static void toggle();
    static bool isOn();
    
    // Các chế độ chớp LED
    static void blink1000ms_3times();
    static void blink500ms_3times();
    static void blink200ms_for_3s();
    
private:
    static void blink(uint32_t delayMs, uint32_t times);
    static SemaphoreHandle_t ledMutex;
    static bool ledState;
};

#endif // LED_CONTROLLER_H
