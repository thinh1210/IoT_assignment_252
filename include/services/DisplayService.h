#ifndef DISPLAY_SERVICE_H
#define DISPLAY_SERVICE_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class DisplayService {
public:
  static void init();

private:
  static TaskHandle_t task_display_handle;

  static void task_display_loop(void *param);
};

#endif // DISPLAY_SERVICE_H
