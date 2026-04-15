#include "services/ManualControlService.h"

#include "config.h"
#include "drivers/LedController.h"
#include "drivers/NeonController.h"
#include "Common/AppLog.h"
#include "services/ApService.h"

static const char *TAG = "ManualControlSvc";

SemaphoreHandle_t ManualControlService::stateMutex = nullptr;
ManualControlSnapshot ManualControlService::state = {false, false, false, false,
                                                     false};

void ManualControlService::init() {
  if (stateMutex == nullptr) {
    stateMutex = xSemaphoreCreateMutex();
  }
}

void ManualControlService::enterManualMode() {
  init();

  if (stateMutex == nullptr ||
      xSemaphoreTake(stateMutex, pdMS_TO_TICKS(50)) != pdPASS) {
    ESP_LOGW(TAG, "Failed to enter MANUAL mode: mutex unavailable");
    return;
  }

  state.active = true;
  state.fanOn = ApService::getRelayState(PLANT_CARE_FAN_RELAY_GPIO, false);
  state.pumpOn = ApService::getRelayState(PLANT_CARE_PUMP_RELAY_GPIO, false);
  state.greenLedOn = LedController::isOn();
  state.neoLedOn = NeonController::hasEnvironmentSignal();
  xSemaphoreGive(stateMutex);

  NeonController::setManualOverride(true, state.neoLedOn);
  ESP_LOGI(TAG,
           "Entered MANUAL mode -> fan=%s pump=%s green=%s neo=%s",
           state.fanOn ? "ON" : "OFF", state.pumpOn ? "ON" : "OFF",
           state.greenLedOn ? "ON" : "OFF", state.neoLedOn ? "ON" : "OFF");
}

void ManualControlService::exitManualMode() {
  init();

  if (stateMutex != nullptr &&
      xSemaphoreTake(stateMutex, pdMS_TO_TICKS(50)) == pdPASS) {
    state.active = false;
    xSemaphoreGive(stateMutex);
  }

  NeonController::setManualOverride(false, false);
  ESP_LOGI(TAG, "Exited MANUAL mode. Neo restored to environment control.");
}

bool ManualControlService::isActive() {
  init();

  bool active = false;
  if (stateMutex != nullptr &&
      xSemaphoreTake(stateMutex, pdMS_TO_TICKS(20)) == pdPASS) {
    active = state.active;
    xSemaphoreGive(stateMutex);
  }
  return active;
}

void ManualControlService::handleRemoteCommand(RemoteCommand command) {
  if (!isActive()) {
    ESP_LOGW(TAG, "Ignoring remote command outside MANUAL mode.");
    return;
  }

  switch (command) {
  case RemoteCommand::FAN_TOGGLE:
    toggleRelay("fan", PLANT_CARE_FAN_RELAY_GPIO, state.fanOn);
    break;
  case RemoteCommand::PUMP_TOGGLE:
    toggleRelay("pump", PLANT_CARE_PUMP_RELAY_GPIO, state.pumpOn);
    break;
  case RemoteCommand::GREEN_LED_TOGGLE:
    toggleGreenLed();
    break;
  case RemoteCommand::NEO_LED_TOGGLE:
    toggleNeoLed();
    break;
  case RemoteCommand::MODE_NEXT:
  case RemoteCommand::NONE:
  default:
    break;
  }
}

ManualControlSnapshot ManualControlService::getSnapshot() {
  init();

  ManualControlSnapshot snapshot = {false, false, false, false, false};
  if (stateMutex != nullptr &&
      xSemaphoreTake(stateMutex, pdMS_TO_TICKS(20)) == pdPASS) {
    snapshot = state;
    xSemaphoreGive(stateMutex);
  }
  return snapshot;
}

void ManualControlService::formatStatusLine(char *buffer, size_t bufferSize) {
  if (buffer == nullptr || bufferSize == 0) {
    return;
  }

  const ManualControlSnapshot snapshot = getSnapshot();
  snprintf(buffer, bufferSize, "F:%d P:%d G:%d N:%d", snapshot.fanOn ? 1 : 0,
           snapshot.pumpOn ? 1 : 0, snapshot.greenLedOn ? 1 : 0,
           snapshot.neoLedOn ? 1 : 0);
}

void ManualControlService::toggleRelay(const char *name, int gpio,
                                       bool &stateField) {
  if (gpio <= 0) {
    ESP_LOGW(TAG, "Manual %s toggle ignored. GPIO not configured.", name);
    return;
  }

  if (stateMutex == nullptr ||
      xSemaphoreTake(stateMutex, pdMS_TO_TICKS(50)) != pdPASS) {
    ESP_LOGW(TAG, "Manual %s toggle skipped. Mutex unavailable.", name);
    return;
  }

  stateField = !stateField;
  const bool nextState = stateField;
  xSemaphoreGive(stateMutex);

  if (ApService::setRelayState(gpio, nextState, true)) {
    ESP_LOGI(TAG, "Manual %s -> %s", name, nextState ? "ON" : "OFF");
  } else {
    ESP_LOGW(TAG, "Failed to drive manual %s relay.", name);
  }
}

void ManualControlService::toggleGreenLed() {
  if (stateMutex == nullptr ||
      xSemaphoreTake(stateMutex, pdMS_TO_TICKS(50)) != pdPASS) {
    ESP_LOGW(TAG, "Manual green LED toggle skipped. Mutex unavailable.");
    return;
  }

  state.greenLedOn = !state.greenLedOn;
  const bool nextState = state.greenLedOn;
  xSemaphoreGive(stateMutex);

  LedController::setState(nextState);
  ESP_LOGI(TAG, "Manual green LED -> %s", nextState ? "ON" : "OFF");
}

void ManualControlService::toggleNeoLed() {
  if (stateMutex == nullptr ||
      xSemaphoreTake(stateMutex, pdMS_TO_TICKS(50)) != pdPASS) {
    ESP_LOGW(TAG, "Manual Neo toggle skipped. Mutex unavailable.");
    return;
  }

  state.neoLedOn = !state.neoLedOn;
  const bool nextState = state.neoLedOn;
  xSemaphoreGive(stateMutex);

  NeonController::setManualOverride(true, nextState);
  ESP_LOGI(TAG, "Manual Neo LED -> %s", nextState ? "ON" : "OFF");
}
