#include "services/PlantCareRuntimeService.h"

#include "Common/AppLog.h"
#include "Common/PlantCareState.h"
#include "Main_FSM/Main_FSM.h"
#include "config.h"
#include "services/ApService.h"
#include "services/PlantCareInferenceService.h"

#include <cmath>
#include <freertos/FreeRTOS.h>

namespace {

static const char *TAG = "PlantCareRuntime";
portMUX_TYPE gPlantCareMux = portMUX_INITIALIZER_UNLOCKED;

PlantCareSnapshot readSnapshotLocked() {
  PlantCareSnapshot snapshot;
  snapshot.ready = globalPlantCareReady;
  snapshot.action = globalPlantCareAction;
  snapshot.confidence = globalPlantCareConfidence;
  return snapshot;
}

void writeSnapshotLocked(const PlantCareSnapshot &snapshot) {
  globalPlantCareReady = snapshot.ready;
  globalPlantCareAction = snapshot.action;
  globalPlantCareConfidence = snapshot.confidence;
}

bool isActionable(const PlantCareSnapshot &snapshot) {
  // In auto modes, any non-zero TinyML action is treated as "do something"
  // and mapped to a fan-on request because fan is the available actuator.
  return snapshot.ready && snapshot.action != 0;
}

const char *toShortLabel(int action) {
  switch (action) {
  case 0:
    return "NONE";
  case 1:
    return "FAN";
  case 2:
    return "WATER";
  default:
    return "UNK";
  }
}

} // namespace

bool PlantCareRuntimeService::updateFromSensor(float temperature, float humidity) {
  int predictedLabel = 0;
  float confidence = 0.0f;
  PlantCareSnapshot nextSnapshot = {false, 0, 0.0f};

  if (!PlantCareInferenceService::predict(temperature, humidity, predictedLabel,
                                          confidence)) {
    ESP_LOGE(TAG, "TinyML prediction failed. T=%.1f C, H=%.0f %%",
             temperature, humidity);
  } else {
    nextSnapshot.ready = true;
    nextSnapshot.action = predictedLabel;
    nextSnapshot.confidence = confidence;
    ESP_LOGI(TAG, "TinyML -> T=%.1f C, H=%.0f %% => %s (id=%d, conf=%.2f)",
             temperature, humidity,
             PlantCareInferenceService::labelToString(predictedLabel),
             predictedLabel, confidence);
  }

  portENTER_CRITICAL(&gPlantCareMux);
  writeSnapshotLocked(nextSnapshot);
  portEXIT_CRITICAL(&gPlantCareMux);

  syncAutomationForMode(Main_FSM::getMode());
  return nextSnapshot.ready;
}

PlantCareSnapshot PlantCareRuntimeService::getSnapshot() {
  portENTER_CRITICAL(&gPlantCareMux);
  const PlantCareSnapshot snapshot = readSnapshotLocked();
  portEXIT_CRITICAL(&gPlantCareMux);
  return snapshot;
}

void PlantCareRuntimeService::syncAutomationForMode(SystemMode mode) {
  if (mode != SystemMode::NORMAL_MODE && mode != SystemMode::ACCESSPOINT_MODE) {
    return;
  }

  if (PLANT_CARE_FAN_RELAY_GPIO <= 0) {
    return;
  }

  const PlantCareSnapshot snapshot = getSnapshot();
  const bool shouldTurnFanOn = isActionable(snapshot);
  const bool currentFanState =
      ApService::getRelayState(PLANT_CARE_FAN_RELAY_GPIO, false);

  if (currentFanState == shouldTurnFanOn) {
    return;
  }

  if (ApService::setRelayState(PLANT_CARE_FAN_RELAY_GPIO, shouldTurnFanOn,
                               false)) {
    ESP_LOGI(TAG, "Automation fan -> %s (mode=%d, action=%d, conf=%.2f)",
             shouldTurnFanOn ? "ON" : "OFF", static_cast<int>(mode),
             snapshot.action, snapshot.confidence);
  } else {
    ESP_LOGW(TAG, "Automation fan update failed. requested=%s",
             shouldTurnFanOn ? "ON" : "OFF");
  }
}

void PlantCareRuntimeService::formatDisplayLine(char *buffer, size_t bufferSize) {
  if (buffer == nullptr || bufferSize == 0) {
    return;
  }

  const PlantCareSnapshot snapshot = getSnapshot();
  if (!snapshot.ready) {
    snprintf(buffer, bufferSize, "AI: WAITING");
    return;
  }

  const long confidencePct =
      lroundf(fmaxf(0.0f, fminf(snapshot.confidence, 1.0f)) * 100.0f);
  snprintf(buffer, bufferSize, "AI:%s %ld%%", toShortLabel(snapshot.action),
           confidencePct);
}
