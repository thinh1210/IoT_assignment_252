#include "drivers/OLEDDisplay.h"
#include "config.h"
#include "Common/AppLog.h"

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

void OLEDDisplay::drawAPPage(int level, const char *statusText,
                             const char *careText) {
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
  u8g2.drawStr(0, 56, statusText);
  u8g2.drawStr(0, 63, careText);
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
                                    const char *statusText,
                                    const char *careText) {
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

  u8g2.setFont(u8g2_font_5x7_tf);
  u8g2.drawStr(50, 63, careText);
}

void OLEDDisplay::drawSuccessPage() {
  int x_off = -5;
  u8g2.setFont(u8g2_font_open_iconic_all_4x_t);
  u8g2.drawGlyph(48 + x_off, 45, 0x0073);

  u8g2.setFont(u8g2_font_ncenB12_tr);
  u8g2.drawStr(15 + x_off, 62, "UPDATE OK!");
}

void OLEDDisplay::playWelcomeAnimation(const char *name) {
  const char *resolvedName =
      (name != nullptr && name[0] != '\0') ? name : "Khoa";

  for (int16_t offsetX = -96; offsetX <= 0; offsetX += 8) {
    drawWelcomeFrame(resolvedName, offsetX, 18, false);
    render();
    delay(40);
  }

  for (uint8_t accentWidth = 22; accentWidth <= 82; accentWidth += 12) {
    drawWelcomeFrame(resolvedName, 0, accentWidth, false);
    render();
    delay(55);
  }

  for (uint8_t pulse = 0; pulse < 4; ++pulse) {
    drawWelcomeFrame(resolvedName, 0, 82, (pulse % 2U) == 0U);
    render();
    delay(120);
  }

  drawWelcomeFrame(resolvedName, 0, 82, false);
  render();
  delay(700);
}

void OLEDDisplay::drawWelcomeFrame(const char *name, int16_t offsetX,
                                   uint8_t accentWidth, bool invertAccent) {
  const int16_t baseX = 18 + offsetX;
  const int16_t cardX = 8;
  const int16_t cardY = 6;
  const uint8_t cardW = 112;
  const uint8_t cardH = 52;

  clear();
  u8g2.drawRFrame(cardX, cardY, cardW, cardH, 6);
  u8g2.drawRFrame(cardX + 2, cardY + 2, cardW - 4, cardH - 4, 5);

  if (invertAccent) {
    u8g2.drawBox(23, 47, accentWidth, 4);
  } else {
    u8g2.drawRBox(23, 47, accentWidth, 4, 2);
  }

  u8g2.setFont(u8g2_font_5x7_tf);
  u8g2.drawStr(baseX, 20, "chao");

  u8g2.setFont(u8g2_font_ncenB12_tr);
  u8g2.drawStr(baseX, 42, name);

  u8g2.setFont(u8g2_font_5x7_tf);
  u8g2.drawStr(89, 20, "*");
  u8g2.drawStr(98, 28, "*");
  u8g2.drawStr(84, 34, "*");
}
