#include "drivers/OLEDDisplay.h"
#include "config.h"

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
}

void OLEDDisplay::clear() { u8g2.clearBuffer(); }

void OLEDDisplay::render() { u8g2.sendBuffer(); }

void OLEDDisplay::drawAPPage(int level) {
  u8g2.setFont(u8g2_font_haxrcorp4089_tr);
  u8g2.drawStr(35, 12, "ACCESS POINT");

  int cx = 64, cy = 40;
  u8g2.drawDisc(cx, cy, 2);
  for (int i = 0; i <= level; i++) {
    u8g2.drawCircle(cx, cy, 10 + i * 8,
                    U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);
  }

  u8g2.drawStr(25, 62, "SSID: ESP32_CONFIG");
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

void OLEDDisplay::drawTelemetryPage(float t, float h) {
  u8g2.setFont(u8g2_font_haxrcorp4089_tr);
  u8g2.drawStr(30, 10, "ENVIRONMENT");
  u8g2.drawHLine(10, 12, 108);

  u8g2.setFont(u8g2_font_ncenB12_tr);
  u8g2.setCursor(15, 35);
  u8g2.print("Temp: ");
  u8g2.print(t, 1);
  u8g2.print(" C");

  u8g2.setCursor(15, 55);
  u8g2.print("Humi: ");
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
