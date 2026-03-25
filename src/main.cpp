#include "config.h"
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

/**
 * 🧪 FEATURE TEST PHASE 1 (RADIAL LOADING): FULL-SCREEN OLED PAGES
 * - Loading: Radial bars spinner (iOS style)
 */

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE,
                                        /* clock=*/I2C_SCL, /* data=*/I2C_SDA);

enum class DisplayPage { ACCESS_POINT, LOADING, TELEMETRY, SUCCESS };

DisplayPage currentPage = DisplayPage::LOADING; // Mặc định vào thẳng Loading để check
int loading_step = 0;
int wifi_level = 0;
uint32_t last_anim_update = 0;

float demoTemp = 28.5;
float demoHumi = 65.2;

// --- DRAWING FUNCTIONS ---

void drawAPPage(int level) {
  u8g2.setFont(u8g2_font_haxrcorp4089_tr);
  u8g2.drawStr(35, 12, "ACCESS POINT");
  int cx = 64, cy = 40;
  u8g2.drawDisc(cx, cy, 2);
  for (int i = 0; i <= level; i++) {
    u8g2.drawCircle(cx, cy, 10 + i * 8, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);
  }
  u8g2.drawStr(25, 62, "SSID: ESP32_CONFIG");
}

void drawLoadingPage(int step) {
  int x_off = -5;
  u8g2.setFont(u8g2_font_haxrcorp4089_tr);
  u8g2.drawStr(45 + x_off, 18, "LOADING...");
  
  // Radial Bars Spinner (iOS Style)
  int cx = 64 + x_off;
  int cy = 42;
  int inner_r = 8;
  int outer_r = 15;
  
  for (int i = 0; i < 12; i++) {
    // Tính toán góc cho mỗi thanh (30 độ mỗi thanh)
    float angle = i * 30.0 * 3.14159 / 180.0;
    
    // Độ sáng/nhạt giả lập bằng cách thay đổi độ dài thanh
    // step xác định thanh nào đang "sáng nhất"
    int diff = (i - step + 12) % 12;
    int current_outer = outer_r;
    
    if (diff == 0) current_outer = outer_r + 3;      // Thanh chính (dài nhất)
    else if (diff == 1 || diff == 11) current_outer = outer_r + 1; // Thanh lân cận
    else if (diff > 5) continue; // Giấu bớt một nửa vòng tròn để tạo hiệu ứng mờ dần
    
    int x1 = cx + cos(angle) * inner_r;
    int y1 = cy + sin(angle) * inner_r;
    int x2 = cx + cos(angle) * current_outer;
    int y2 = cy + sin(angle) * current_outer;
    
    u8g2.drawLine(x1, y1, x2, y2);
  }
  
  u8g2.drawStr(32 + x_off, 63, "Please wait...");
}

void drawTelemetryPage(float t, float h) {
  u8g2.setFont(u8g2_font_haxrcorp4089_tr);
  u8g2.drawStr(30, 10, "ENVIRONMENT");
  u8g2.drawHLine(10, 12, 108);
  u8g2.setFont(u8g2_font_ncenB12_tr);
  u8g2.setCursor(15, 35);
  u8g2.print("Temp: "); u8g2.print(t, 1); u8g2.print(" C");
  u8g2.setCursor(15, 55);
  u8g2.print("Humi: "); u8g2.print(h, 0); u8g2.print(" %");
}

void drawSuccessPage() {
  int x_off = -5;
  u8g2.setFont(u8g2_font_open_iconic_all_4x_t);
  u8g2.drawGlyph(48 + x_off, 45, 0x0073);
  u8g2.setFont(u8g2_font_ncenB12_tr);
  u8g2.drawStr(15 + x_off, 62, "UPDATE OK!");
}

void setup() {
  Serial.begin(115200);
  u8g2.begin();
  pinMode(BUTTON_2_GPIO, INPUT_PULLUP);
}

void loop() {
  static bool lastBtnState = HIGH;
  bool currentBtnState = digitalRead(BUTTON_2_GPIO);
  if (lastBtnState == HIGH && currentBtnState == LOW) {
    int next = (int)currentPage + 1;
    if (next > 3) next = 0;
    currentPage = (DisplayPage)next;
    delay(50);
  }
  lastBtnState = currentBtnState;

  uint32_t now = millis();
  if (now - last_anim_update > 80) { // Tốc độ spinner chậm lại một chút để nhìn rõ thanh
    loading_step = (loading_step + 1) % 12; 
    wifi_level = (wifi_level + 1) % 4;
    last_anim_update = now;
  }

  u8g2.clearBuffer();
  switch (currentPage) {
    case DisplayPage::ACCESS_POINT: drawAPPage(wifi_level); break;
    case DisplayPage::LOADING:      drawLoadingPage(loading_step); break;
    case DisplayPage::TELEMETRY:    drawTelemetryPage(demoTemp, demoHumi); break;
    case DisplayPage::SUCCESS:      drawSuccessPage(); break;
  }
  u8g2.sendBuffer();
}
