#include "services/CoreIotPublishService.h"

#include "Common/PlantCareState.h"
#include "config.h"
#include "services/ApService.h"
#include "services/PlantCareInferenceService.h"

namespace {

const CoreIotTelemetryKeys kDefaultTelemetryKeys = {
    "temperature",
    "humidity",
    "alert_status",
    "fan_state",
    "pump_state",
    "tinyml_result",
    "tinyml_action_id",
    "tinyml_confidence",
    "care_action",
    "care_action_id",
    "care_confidence",
};

bool publishKeyValue(ThingsBoard &tb, const char *key, float value) {
  return tb.sendTelemetryData(key, value);
}

bool publishKeyValue(ThingsBoard &tb, const char *key, int value) {
  return tb.sendTelemetryData(key, value);
}

bool publishKeyValue(ThingsBoard &tb, const char *key, bool value) {
  return tb.sendTelemetryData(key, value);
}

bool publishKeyValue(ThingsBoard &tb, const char *key, const char *value) {
  return tb.sendTelemetryData(key, value);
}

} // namespace

CoreIotTelemetrySnapshot CoreIotPublishService::buildSnapshot(float temp,
                                                              float humi) {
  CoreIotTelemetrySnapshot snapshot;
  snapshot.temperature = temp;
  snapshot.humidity = humi;
  snapshot.alertStatus = (temp >= COREIOT_ALERT_TEMP_THRESHOLD) ||
                         (humi <= COREIOT_ALERT_HUMI_THRESHOLD);
  snapshot.fanState = ApService::getRelayState(PLANT_CARE_FAN_RELAY_GPIO, false);
  snapshot.pumpState =
      ApService::getRelayState(PLANT_CARE_PUMP_RELAY_GPIO, false);
  snapshot.tinyMlResult = "Unknown";
  snapshot.tinyMlActionId = -1;
  snapshot.tinyMlConfidence = 0.0f;

  if (globalPlantCareReady) {
    snapshot.tinyMlActionId = globalPlantCareAction;
    snapshot.tinyMlConfidence = globalPlantCareConfidence;
    snapshot.tinyMlResult =
        PlantCareInferenceService::labelToString(globalPlantCareAction);
  }

  return snapshot;
}

bool CoreIotPublishService::publish(ThingsBoard &tb,
                                    const CoreIotTelemetrySnapshot &snapshot) {
  const CoreIotTelemetryKeys &keys = defaults();

  bool success = true;
  success &= publishKeyValue(tb, keys.temperature, snapshot.temperature);
  success &= publishKeyValue(tb, keys.humidity, snapshot.humidity);
  success &= publishKeyValue(tb, keys.alertStatus, snapshot.alertStatus);
  success &= publishKeyValue(tb, keys.fanState, snapshot.fanState);
  success &= publishKeyValue(tb, keys.pumpState, snapshot.pumpState);
  success &= publishKeyValue(tb, keys.tinyMlResult, snapshot.tinyMlResult.c_str());
  success &= publishKeyValue(tb, keys.tinyMlActionId, snapshot.tinyMlActionId);
  success &= publishKeyValue(tb, keys.tinyMlConfidence, snapshot.tinyMlConfidence);

  success &= publishKeyValue(tb, keys.legacyCareAction,
                             snapshot.tinyMlResult.c_str());
  success &= publishKeyValue(tb, keys.legacyCareActionId,
                             snapshot.tinyMlActionId);
  success &= publishKeyValue(tb, keys.legacyCareConfidence,
                             snapshot.tinyMlConfidence);
  return success;
}

const CoreIotTelemetryKeys &CoreIotPublishService::defaults() {
  return kDefaultTelemetryKeys;
}
