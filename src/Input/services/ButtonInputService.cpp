#include "Input/services/ButtonInputService.h"
#include "esp_log.h"

static const char *TAG = "ButtonInputSvc";

void ButtonInputService::task_button_poll(void *param) {
  ButtonHandler *handler = (ButtonHandler *)param;
  while (1) {
    if (handler != NULL) {
      handler->loop();
    }
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

// co the them 1 so event de handle o day
void ButtonInputService::handleButtonEvents(ButtonHandler *btnHandler,
                                            QueueHandle_t *qProcessing) {
  ButtonEvent lastEvt = ButtonEvent::NONE;
  int btnIdx = -1;

  while ((btnIdx = btnHandler->getNextEvent(lastEvt)) != -1) {
    switch (lastEvt) {
    case ButtonEvent::SINGLE_CLICK:
      ESP_LOGI(TAG, "BTN %d: SINGLE CLICK (RELEASED)", btnIdx);
      break;
    case ButtonEvent::DOUBLE_CLICK:
      ESP_LOGI(TAG, "BTN %d: DOUBLE CLICK (RELEASED)", btnIdx);
      break;
    case ButtonEvent::TRIPLE_CLICK:
      ESP_LOGI(TAG, "BTN %d: TRIPLE CLICK (RELEASED)", btnIdx);
      break;
    case ButtonEvent::LONG_PRESS:
      ESP_LOGI(TAG, "BTN %d: LONG PRESS (RELEASED)", btnIdx);
      break;
    default:
      break;
    }

    SystemEvent event;
    if (lastEvt == ButtonEvent::LONG_PRESS) {
      event.type = EventType::MODE_CHANGE;
    } else {
      event.type = EventType::BUTTON_PRESSED;
    }

    if (qProcessing != NULL) {
      xQueueSend(*qProcessing, &event, 0);
    }
  }
}
