#include "drivers/NeonController.h"
#include "esp_log.h"

static const char *TAG = "NeonController";

Adafruit_NeoPixel NeonController::strip(NEO_NUM_PIXELS, NEO_LED_GPIO,
                                        NEO_GRB + NEO_KHZ800);
int NeonController::colorIndex = 0;
TaskHandle_t NeonController::blinkTaskHandle = NULL;
SemaphoreHandle_t NeonController::stateMutex = NULL;
NeonController::RGB NeonController::environmentColor = {0, 0, 0};
uint32_t NeonController::environmentBlinkHalfPeriodMs = 250;
bool NeonController::environmentBlinkEnabled = false;
NeonController::EnvironmentLedState NeonController::envState =
    NeonController::EnvironmentLedState::UNKNOWN;
bool NeonController::manualOverrideEnabled = false;
bool NeonController::manualLedOn = false;
NeonController::RGB NeonController::manualColor = {255, 255, 255};

const NeonController::RGB NeonController::colors[] = {
    {255, 0, 0},    // Red
    {0, 255, 0},    // Green
    {0, 0, 255},    // Blue
    {255, 255, 0},  // Yellow (RG)
    {0, 255, 255},  // Cyan (GB)
    {255, 0, 255},  // Magenta (RB)
    {255, 255, 255} // White
};
const int NeonController::numColors = sizeof(colors) / sizeof(colors[0]);

void NeonController::init() {
  if (NEO_LED_GPIO != (gpio_num_t)-1) {
    if (stateMutex == NULL) {
      stateMutex = xSemaphoreCreateMutex();
    }

    strip.begin();
    strip.setBrightness(50); // Mặc định sáng vừa phải
    showColor(0, 0, 0);      // Tắt ban đầu

    if (blinkTaskHandle == NULL) {
      xTaskCreate(task_blink_loop, "neo_blink", 4096, NULL, 3,
                  &blinkTaskHandle);
    }

    ESP_LOGI(TAG, "Neon Controller initialized on GPIO %d", NEO_LED_GPIO);
  }
}

void NeonController::setNextColor() {
  colorIndex = (colorIndex + 1) % numColors;
  RGB c = colors[colorIndex];
  showColor(c.r, c.g, c.b);
  ESP_LOGI(TAG, "Changed Neon color to Index %d (R:%d, G:%d, B:%d)", colorIndex,
           c.r, c.g, c.b);
}

void NeonController::showColor(uint8_t r, uint8_t g, uint8_t b) {
  if (NEO_LED_GPIO == (gpio_num_t)-1) return;
  strip.setPixelColor(0, strip.Color(r, g, b));
  strip.show();
}

void NeonController::updateEnvironment(float temp, float humi) {
  if (NEO_LED_GPIO == (gpio_num_t)-1 || stateMutex == NULL) return;

  const EnvironmentLedState nextState = classify(temp, humi);
  const RGB nextColor = colorForState(nextState);
  const uint32_t nextHalfPeriod = halfPeriodForTemperature(temp);

  if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(50)) == pdPASS) {
    const bool stateChanged =
        (envState != nextState) ||
        (environmentColor.r != nextColor.r) ||
        (environmentColor.g != nextColor.g) ||
        (environmentColor.b != nextColor.b) ||
        (environmentBlinkHalfPeriodMs != nextHalfPeriod) ||
        (environmentBlinkEnabled != (nextState != EnvironmentLedState::UNKNOWN));

    envState = nextState;
    environmentColor = nextColor;
    environmentBlinkHalfPeriodMs = nextHalfPeriod;
    environmentBlinkEnabled = (nextState != EnvironmentLedState::UNKNOWN);
    xSemaphoreGive(stateMutex);

    if (stateChanged) {
      const uint32_t frequencyHz = 1000U / (nextHalfPeriod * 2U);
      ESP_LOGI(TAG,
               "Environment LED updated -> state=%s, color=(%u,%u,%u), blink=%u Hz, T=%.1f C, H=%.0f %%",
               stateToString(nextState), nextColor.r, nextColor.g, nextColor.b,
               frequencyHz, temp, humi);
    }
  }
}

void NeonController::setManualOverride(bool enabled, bool on) {
  if (NEO_LED_GPIO == (gpio_num_t)-1 || stateMutex == NULL) return;

  if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(50)) == pdPASS) {
    const bool changed =
        (manualOverrideEnabled != enabled) || (manualLedOn != on);
    manualOverrideEnabled = enabled;
    manualLedOn = on;
    xSemaphoreGive(stateMutex);

    if (changed) {
      ESP_LOGI(TAG, "Manual override -> enabled=%s, neo=%s",
               enabled ? "true" : "false", on ? "ON" : "OFF");
    }
  }
}

bool NeonController::isManualOn() {
  if (stateMutex == NULL) {
    return manualOverrideEnabled && manualLedOn;
  }

  bool on = false;
  if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(20)) == pdPASS) {
    on = manualOverrideEnabled && manualLedOn;
    xSemaphoreGive(stateMutex);
  }
  return on;
}

bool NeonController::hasEnvironmentSignal() {
  if (stateMutex == NULL) {
    return envState != EnvironmentLedState::UNKNOWN;
  }

  bool active = false;
  if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(20)) == pdPASS) {
    active = envState != EnvironmentLedState::UNKNOWN;
    xSemaphoreGive(stateMutex);
  }
  return active;
}

void NeonController::task_blink_loop(void *param) {
  bool ledOn = false;

  ESP_LOGI(TAG, "Neon blink task started.");
  while (1) {
    RGB color = {0, 0, 0};
    uint32_t halfPeriodMs = 250;
    bool enabled = false;
    bool manualEnabled = false;
    bool manualOn = false;

    if (stateMutex != NULL &&
        xSemaphoreTake(stateMutex, pdMS_TO_TICKS(20)) == pdPASS) {
      color = environmentColor;
      halfPeriodMs = environmentBlinkHalfPeriodMs;
      enabled = environmentBlinkEnabled;
      manualEnabled = manualOverrideEnabled;
      manualOn = manualLedOn;
      xSemaphoreGive(stateMutex);
    }

    if (manualEnabled) {
      if (manualOn) {
        showColor(manualColor.r, manualColor.g, manualColor.b);
      } else if (ledOn) {
        showColor(0, 0, 0);
      }
      ledOn = manualOn;
      vTaskDelay(pdMS_TO_TICKS(100));
      continue;
    }

    if (!enabled) {
      if (ledOn) {
        showColor(0, 0, 0);
        ledOn = false;
      }
      vTaskDelay(pdMS_TO_TICKS(250));
      continue;
    }

    if (ledOn) {
      showColor(0, 0, 0);
    } else {
      showColor(color.r, color.g, color.b);
    }
    ledOn = !ledOn;
    vTaskDelay(pdMS_TO_TICKS(halfPeriodMs));
  }
}

NeonController::RGB
NeonController::colorForState(EnvironmentLedState state) {
  switch (state) {
  case EnvironmentLedState::NORMAL:
    return {0, 255, 0};
  case EnvironmentLedState::WARNING:
    return {255, 180, 0};
  case EnvironmentLedState::CRITICAL:
    return {255, 0, 0};
  case EnvironmentLedState::UNKNOWN:
  default:
    return {0, 0, 0};
  }
}

NeonController::EnvironmentLedState
NeonController::classify(float temp, float humi) {
  if (humi <= 0.0f) {
    return EnvironmentLedState::UNKNOWN;
  }

  if (temp >= ENV_TEMP_CRITICAL_THRESHOLD ||
      humi <= ENV_HUMI_CRITICAL_LOW_THRESHOLD ||
      humi >= ENV_HUMI_CRITICAL_HIGH_THRESHOLD) {
    return EnvironmentLedState::CRITICAL;
  }

  if (temp >= ENV_TEMP_WARNING_THRESHOLD ||
      humi <= ENV_HUMI_WARNING_LOW_THRESHOLD) {
    return EnvironmentLedState::WARNING;
  }

  return EnvironmentLedState::NORMAL;
}

uint32_t NeonController::halfPeriodForTemperature(float temp) {
  if (temp < ENV_TEMP_BLINK_LOW_THRESHOLD) {
    return 500; // 1Hz
  }
  if (temp > ENV_TEMP_BLINK_HIGH_THRESHOLD) {
    return 100; // 5Hz
  }
  return 250; // 2Hz
}

const char *NeonController::stateToString(EnvironmentLedState state) {
  switch (state) {
  case EnvironmentLedState::NORMAL:
    return "NORMAL";
  case EnvironmentLedState::WARNING:
    return "WARNING";
  case EnvironmentLedState::CRITICAL:
    return "CRITICAL";
  case EnvironmentLedState::UNKNOWN:
  default:
    return "UNKNOWN";
  }
}
