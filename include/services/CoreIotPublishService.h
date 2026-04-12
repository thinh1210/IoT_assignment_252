#ifndef CORE_IOT_PUBLISH_SERVICE_H
#define CORE_IOT_PUBLISH_SERVICE_H

#include <Arduino.h>
#include <ThingsBoard.h>

struct CoreIotTelemetrySnapshot {
  float temperature;
  float humidity;
  bool alertStatus;
  bool fanState;
  bool pumpState;
  String tinyMlResult;
  int tinyMlActionId;
  float tinyMlConfidence;
};

struct CoreIotTelemetryKeys {
  const char *temperature;
  const char *humidity;
  const char *alertStatus;
  const char *fanState;
  const char *pumpState;
  const char *tinyMlResult;
  const char *tinyMlActionId;
  const char *tinyMlConfidence;
  const char *legacyCareAction;
  const char *legacyCareActionId;
  const char *legacyCareConfidence;
};

class CoreIotPublishService {
public:
  static CoreIotTelemetrySnapshot buildSnapshot(float temp, float humi);
  static bool publish(ThingsBoard &tb,
                      const CoreIotTelemetrySnapshot &snapshot);
  static const CoreIotTelemetryKeys &defaults();
};

#endif // CORE_IOT_PUBLISH_SERVICE_H
