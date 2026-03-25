#ifndef BUTTON_INPUT_SERVICE_H
#define BUTTON_INPUT_SERVICE_H

#include "drivers/ButtonHandler.h"
#include "Common/Events.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

class ButtonInputService {
public:
  static void task_button_poll(void *param);
  static void handleButtonEvents(ButtonHandler *btnHandler, QueueHandle_t *qProcessing);
};

#endif
