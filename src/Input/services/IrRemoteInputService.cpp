#include "Input/services/IrRemoteInputService.h"

#include "config.h"
#include "esp_log.h"

#define NO_LED_RECEIVE_FEEDBACK_CODE
#include <IRremote.hpp>

static const char *TAG = "IrRemoteInputSvc";

namespace {

RemoteCommand commandFromRawCode(uint64_t rawCode) {
  if (IR_REMOTE_CODE_MODE_NEXT != 0ULL &&
      rawCode == static_cast<uint64_t>(IR_REMOTE_CODE_MODE_NEXT)) {
    return RemoteCommand::MODE_NEXT;
  }
  if (IR_REMOTE_CODE_FAN_TOGGLE != 0ULL &&
      rawCode == static_cast<uint64_t>(IR_REMOTE_CODE_FAN_TOGGLE)) {
    return RemoteCommand::FAN_TOGGLE;
  }
  if (IR_REMOTE_CODE_PUMP_TOGGLE != 0ULL &&
      rawCode == static_cast<uint64_t>(IR_REMOTE_CODE_PUMP_TOGGLE)) {
    return RemoteCommand::PUMP_TOGGLE;
  }
  if (IR_REMOTE_CODE_GREEN_LED_TOGGLE != 0ULL &&
      rawCode == static_cast<uint64_t>(IR_REMOTE_CODE_GREEN_LED_TOGGLE)) {
    return RemoteCommand::GREEN_LED_TOGGLE;
  }
  if (IR_REMOTE_CODE_NEO_LED_TOGGLE != 0ULL &&
      rawCode == static_cast<uint64_t>(IR_REMOTE_CODE_NEO_LED_TOGGLE)) {
    return RemoteCommand::NEO_LED_TOGGLE;
  }
  return RemoteCommand::NONE;
}

const char *commandToString(RemoteCommand command) {
  switch (command) {
  case RemoteCommand::MODE_NEXT:
    return "MODE_NEXT";
  case RemoteCommand::FAN_TOGGLE:
    return "FAN_TOGGLE";
  case RemoteCommand::PUMP_TOGGLE:
    return "PUMP_TOGGLE";
  case RemoteCommand::GREEN_LED_TOGGLE:
    return "GREEN_LED_TOGGLE";
  case RemoteCommand::NEO_LED_TOGGLE:
    return "NEO_LED_TOGGLE";
  case RemoteCommand::NONE:
  default:
    return "NONE";
  }
}

bool shouldSuppress(uint64_t rawCode, bool isRepeat) {
  static uint64_t lastCode = 0ULL;
  static uint32_t lastHandledAt = 0U;

  const uint32_t now = millis();
  const bool withinDebounce =
      (rawCode == lastCode) && ((now - lastHandledAt) < IR_REMOTE_DEBOUNCE_MS);

  if (!isRepeat && !withinDebounce) {
    lastCode = rawCode;
    lastHandledAt = now;
    return false;
  }

  return true;
}

void logLearnFrame(uint64_t rawCode) {
#if IR_REMOTE_ENABLE_LEARN_LOG
  Serial.printf("[IR-LEARN] raw=0x%llX\n",
                static_cast<unsigned long long>(rawCode));
  IrReceiver.printIRResultShort(&Serial);
  IrReceiver.printIRSendUsage(&Serial);
#else
  (void)rawCode;
#endif
}

} // namespace

void IrRemoteInputService::task_ir_remote_poll(void *param) {
  QueueHandle_t *qProcessing = static_cast<QueueHandle_t *>(param);

  IrReceiver.begin(IR_RECEIVER_GPIO, DISABLE_LED_FEEDBACK);
  ESP_LOGI(TAG, "IR remote task started on GPIO %d. Learn log=%s",
           IR_RECEIVER_GPIO, IR_REMOTE_ENABLE_LEARN_LOG ? "ON" : "OFF");

  while (1) {
    if (!IrReceiver.decode()) {
      vTaskDelay(pdMS_TO_TICKS(20));
      continue;
    }

    const uint64_t rawCode =
        static_cast<uint64_t>(IrReceiver.decodedIRData.decodedRawData);
    const bool isRepeat =
        (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) != 0;
    const RemoteCommand command = commandFromRawCode(rawCode);
    const bool suppressed = shouldSuppress(rawCode, isRepeat);

    if (command == RemoteCommand::NONE) {
      if (!suppressed) {
        ESP_LOGI(TAG, "IR unmapped frame received. raw=0x%llX repeat=%s",
                 static_cast<unsigned long long>(rawCode),
                 isRepeat ? "true" : "false");
        logLearnFrame(rawCode);
      }
    } else if (!suppressed && qProcessing != nullptr) {
      SystemEvent event{};
      event.type = EventType::IR_REMOTE_CMD;
      event.remote_command = command;
      event.remote_raw_code = rawCode;

      if (xQueueSend(*qProcessing, &event, 0) == pdPASS) {
        ESP_LOGI(TAG, "Queued IR command %s (raw=0x%llX)",
                 commandToString(command),
                 static_cast<unsigned long long>(rawCode));
      } else {
        ESP_LOGW(TAG, "Failed to queue IR command %s",
                 commandToString(command));
      }
    }

    IrReceiver.resume();
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}
