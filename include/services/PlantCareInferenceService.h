#ifndef PLANT_CARE_INFERENCE_SERVICE_H
#define PLANT_CARE_INFERENCE_SERVICE_H

#include <Arduino.h>

class PlantCareInferenceService {
public:
  static bool init();
  static bool predict(float temperature, float humidity, int &predictedLabel,
                      float &confidence);
  static const char *labelToString(int predictedLabel);

private:
  static int8_t quantizeFeature(float value, float mean, float stdDev);
  static float dequantizeOutput(int8_t value);
};

#endif // PLANT_CARE_INFERENCE_SERVICE_H
