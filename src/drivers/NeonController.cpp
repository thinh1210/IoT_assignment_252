#include "drivers/NeonController.h"
#include "esp_log.h"

static const char *TAG = "NeonController";

Adafruit_NeoPixel NeonController::strip(NEO_NUM_PIXELS, NEO_LED_GPIO,
                                        NEO_GRB + NEO_KHZ800);
int NeonController::colorIndex = 0;

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
    strip.begin();
    strip.setBrightness(50); // Mặc định sáng vừa phải
    showColor(0, 0, 0);      // Tắt ban đầu
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
  strip.setPixelColor(0, strip.Color(r, g, b));
  strip.show();
}
