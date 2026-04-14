#ifndef DISPLAY_SERVICE_H
#define DISPLAY_SERVICE_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class DisplayService {
public:
  static void init();
  static void showNormalMode(float temp, float humi);
  static void showAPMode(int level);
  static void showManualMode();

private:
  enum class DisplayView { NORMAL, AP, MANUAL };

  static TaskHandle_t task_display_handle;
  static volatile DisplayView currentView;
  static volatile int apSignalLevel;

  static void task_display_loop(void *param);
};

#endif // DISPLAY_SERVICE_H
