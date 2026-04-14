#include "drivers/OLEDDisplay.h"
#include "config.h"
#include "esp_log.h"

static const char *TAG = "OLEDDisplay";

OLEDDisplay::OLEDDisplay()
    : u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE, /* clock=*/I2C_SCL,
           /* data=*/I2C_SDA) {}

OLEDDisplay& OLEDDisplay::getInstance() {
  static OLEDDisplay instance;
  return instance;
}

void OLEDDisplay::init() {
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_haxrcorp4089_tr);
  u8g2.drawStr(0, 10, "OLED Driver Init...");
  u8g2.sendBuffer();
  ESP_LOGI(TAG, "OLED initialized on I2C SDA=%d SCL=%d", I2C_SDA, I2C_SCL);
}

void OLEDDisplay::clear() { u8g2.clearBuffer(); }

void OLEDDisplay::render() { u8g2.sendBuffer(); }

void OLEDDisplay::drawAPPage(int level, const char *statusText) {
  u8g2.setFont(u8g2_font_5x7_tf);
  u8g2.drawStr(0, 8, "MODE: ACCESS POINT");
  u8g2.drawStr(0, 16, "PORTAL READY");

  int cx = 64, cy = 35;
  u8g2.drawDisc(cx, cy, 2);
  for (int i = 0; i <= level; i++) {
    u8g2.drawCircle(cx, cy, 10 + i * 8,
                    U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);
  }

  u8g2.setCursor(0, 54);
  u8g2.print("SSID: ");
  u8g2.print(ACCESSPOINT_SSID);
  u8g2.drawStr(0, 63, statusText);
}

void OLEDDisplay::drawLoadingPage(int step) {
  int x_off = -5;
  u8g2.setFont(u8g2_font_haxrcorp4089_tr);
  u8g2.drawStr(45 + x_off, 18, "LOADING...");

  // Radial Bars Spinner (iOS Style)
  int cx = 64 + x_off;
  int cy = 42;
  int inner_r = 8;
  int outer_r = 15;

  for (int i = 0; i < 12; i++) {
    float angle = i * 30.0 * 3.14159 / 180.0;
    int diff = (i - step + 12) % 12;
    int current_outer = outer_r;

    if (diff == 0)
      current_outer = outer_r + 3;
    else if (diff == 1 || diff == 11)
      current_outer = outer_r + 1;
    else if (diff > 5)
      continue;

    int x1 = cx + cos(angle) * inner_r;
    int y1 = cy + sin(angle) * inner_r;
    int x2 = cx + cos(angle) * current_outer;
    int y2 = cy + sin(angle) * current_outer;

    u8g2.drawLine(x1, y1, x2, y2);
  }

  u8g2.drawStr(32 + x_off, 63, "Please wait...");
}

void OLEDDisplay::drawTelemetryPage(float t, float h, const char *modeText,
                                    const char *statusText) {
  u8g2.setFont(u8g2_font_5x7_tf);
  u8g2.drawStr(0, 8, modeText);
  u8g2.drawStr(0, 16, statusText);
  u8g2.drawHLine(0, 18, 128);

  u8g2.setFont(u8g2_font_ncenB12_tr);
  u8g2.setCursor(6, 38);
  u8g2.print("T: ");
  u8g2.print(t, 1);
  u8g2.print(" C");

  u8g2.setCursor(6, 58);
  u8g2.print("H: ");
  u8g2.print(h, 0);
  u8g2.print(" %");
}

void OLEDDisplay::drawSuccessPage() {
  int x_off = -5;
  u8g2.setFont(u8g2_font_open_iconic_all_4x_t);
  u8g2.drawGlyph(48 + x_off, 45, 0x0073);

  u8g2.setFont(u8g2_font_ncenB12_tr);
  u8g2.drawStr(15 + x_off, 62, "UPDATE OK!");
}
