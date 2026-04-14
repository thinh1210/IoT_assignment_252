#ifndef NEON_CONTROLLER_H
#define NEON_CONTROLLER_H

#include <Adafruit_NeoPixel.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include "config.h"

#define NEO_NUM_PIXELS 1

class NeonController {
public:
    static void init();
    static void setNextColor();
    static void showColor(uint8_t r, uint8_t g, uint8_t b);
    static void updateEnvironment(float temp, float humi);
    static void setManualOverride(bool enabled, bool on);
    static bool isManualOn();
    static bool hasEnvironmentSignal();

private:
    enum class EnvironmentLedState {
        UNKNOWN,
        NORMAL,
        WARNING,
        CRITICAL
    };

    struct RGB {
        uint8_t r, g, b;
    };

    static Adafruit_NeoPixel strip;
    static int colorIndex;
    static TaskHandle_t blinkTaskHandle;
    static SemaphoreHandle_t stateMutex;
    static RGB environmentColor;
    static uint32_t environmentBlinkHalfPeriodMs;
    static bool environmentBlinkEnabled;
    static EnvironmentLedState envState;
    static bool manualOverrideEnabled;
    static bool manualLedOn;
    static RGB manualColor;

    static const RGB colors[];
    static const int numColors;
    static void task_blink_loop(void *param);
    static RGB colorForState(EnvironmentLedState state);
    static EnvironmentLedState classify(float temp, float humi);
    static uint32_t halfPeriodForTemperature(float temp);
    static const char *stateToString(EnvironmentLedState state);
};

#endif // NEON_CONTROLLER_H
