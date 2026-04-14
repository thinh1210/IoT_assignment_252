#ifndef MANUAL_CONTROL_SERVICE_H
#define MANUAL_CONTROL_SERVICE_H

#include "Common/Events.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

struct ManualControlSnapshot {
  bool active;
  bool fanOn;
  bool pumpOn;
  bool greenLedOn;
  bool neoLedOn;
};

class ManualControlService {
public:
  static void init();
  static void enterManualMode();
  static void exitManualMode();
  static bool isActive();
  static void handleRemoteCommand(RemoteCommand command);
  static ManualControlSnapshot getSnapshot();
  static void formatStatusLine(char *buffer, size_t bufferSize);

private:
  static SemaphoreHandle_t stateMutex;
  static ManualControlSnapshot state;

  static void toggleRelay(const char *name, int gpio, bool &stateField);
  static void toggleGreenLed();
  static void toggleNeoLed();
};

#endif // MANUAL_CONTROL_SERVICE_H
