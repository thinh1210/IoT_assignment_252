#include "services/PlantCareInferenceService.h"

#include "tinyml_model_data.h"

#include "esp_log.h"
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/core/api/error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include <cmath>
#include <new>

namespace {

static const char *TAG = "PlantCareML";
constexpr int kTensorArenaSize = 8 * 1024;

alignas(16) uint8_t gTensorArena[kTensorArenaSize];
alignas(tflite::MicroInterpreter) uint8_t gInterpreterStorage[sizeof(tflite::MicroInterpreter)];

tflite::MicroInterpreter *gInterpreter = nullptr;
TfLiteTensor *gInputTensor = nullptr;
TfLiteTensor *gOutputTensor = nullptr;
bool gInitAttempted = false;
bool gInitSucceeded = false;

} // namespace

bool PlantCareInferenceService::init() {
  if (gInitAttempted) {
    return gInitSucceeded;
  }
  gInitAttempted = true;

  static tflite::MicroMutableOpResolver<2> resolver;
  static bool resolverReady = false;

  const tflite::Model *model = tflite::GetModel(tinyml::kModelData);
  if (model == nullptr) {
    ESP_LOGE(TAG, "Failed to map TinyML model.");
    return false;
  }

  if (model->version() != TFLITE_SCHEMA_VERSION) {
    ESP_LOGE(TAG, "Model schema version %d is not supported (%d).",
             model->version(), TFLITE_SCHEMA_VERSION);
    return false;
  }

  if (!resolverReady) {
    if (resolver.AddFullyConnected() != kTfLiteOk ||
        resolver.AddSoftmax() != kTfLiteOk) {
      ESP_LOGE(TAG, "Failed to register required TFLite Micro ops.");
      return false;
    }
    resolverReady = true;
  }

  gInterpreter = new (gInterpreterStorage)
      tflite::MicroInterpreter(model, resolver, gTensorArena, kTensorArenaSize);

  if (gInterpreter->AllocateTensors() != kTfLiteOk) {
    ESP_LOGE(TAG, "AllocateTensors() failed. Increase tensor arena size.");
    return false;
  }

  gInputTensor = gInterpreter->input(0);
  gOutputTensor = gInterpreter->output(0);

  if (gInputTensor == nullptr || gOutputTensor == nullptr) {
    ESP_LOGE(TAG, "Interpreter tensors are not available.");
    return false;
  }

  if (gInputTensor->type != kTfLiteInt8 || gOutputTensor->type != kTfLiteInt8) {
    ESP_LOGE(TAG, "Expected int8 input/output tensors, got input=%d output=%d.",
             gInputTensor->type, gOutputTensor->type);
    return false;
  }

  ESP_LOGI(TAG, "TinyML interpreter ready. Arena used: %d bytes",
           gInterpreter->arena_used_bytes());

  gInitSucceeded = true;
  return true;
}

bool PlantCareInferenceService::predict(float temperature, float humidity,
                                        int &predictedLabel,
                                        float &confidence) {
  if (!init()) {
    return false;
  }

  gInputTensor->data.int8[0] =
      quantizeFeature(temperature, tinyml::kInputMean[0], tinyml::kInputStd[0]);
  gInputTensor->data.int8[1] =
      quantizeFeature(humidity, tinyml::kInputMean[1], tinyml::kInputStd[1]);

  if (gInterpreter->Invoke() != kTfLiteOk) {
    ESP_LOGE(TAG, "Invoke() failed during plant-care inference.");
    return false;
  }

  predictedLabel = 0;
  confidence = 0.0f;

  for (int index = 0; index < 3; ++index) {
    const float probability = dequantizeOutput(gOutputTensor->data.int8[index]);
    if (probability > confidence) {
      confidence = probability;
      predictedLabel = index;
    }
  }

  return true;
}

const char *PlantCareInferenceService::labelToString(int predictedLabel) {
  switch (predictedLabel) {
  case 0:
    return "No action";
  case 1:
    return "Turn on fan";
  case 2:
    return "Need watering";
  default:
    return "Unknown";
  }
}

int8_t PlantCareInferenceService::quantizeFeature(float value, float mean,
                                                  float stdDev) {
  const float normalized = (value - mean) / stdDev;
  const float scaled =
      (normalized / tinyml::kInputScale) + static_cast<float>(tinyml::kInputZeroPoint);
  long quantized = lroundf(scaled);

  if (quantized < -128) {
    quantized = -128;
  } else if (quantized > 127) {
    quantized = 127;
  }

  return static_cast<int8_t>(quantized);
}

float PlantCareInferenceService::dequantizeOutput(int8_t value) {
  float probability =
      (static_cast<int>(value) - tinyml::kOutputZeroPoint) * tinyml::kOutputScale;

  if (probability < 0.0f) {
    probability = 0.0f;
  } else if (probability > 1.0f) {
    probability = 1.0f;
  }

  return probability;
}
