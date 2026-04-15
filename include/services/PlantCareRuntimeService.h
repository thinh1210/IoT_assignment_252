#ifndef PLANT_CARE_RUNTIME_SERVICE_H
#define PLANT_CARE_RUNTIME_SERVICE_H

#include "Common/Events.h"

#include <Arduino.h>

struct PlantCareSnapshot {
  bool ready;
  int action;
  float confidence;
};

class PlantCareRuntimeService {
public:
  static bool updateFromSensor(float temperature, float humidity);
  static PlantCareSnapshot getSnapshot();
  static void syncAutomationForMode(SystemMode mode);
  static void formatDisplayLine(char *buffer, size_t bufferSize);
};

#endif // PLANT_CARE_RUNTIME_SERVICE_H
