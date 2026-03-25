#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

class OLEDDisplay {
public:
  // Singleton instance
  static OLEDDisplay& getInstance();

  void init();
  void clear();
  void render();

  // Screen Drawing Methods
  void drawAPPage(int level);
  void drawLoadingPage(int step);
  void drawTelemetryPage(float t, float h);
  void drawSuccessPage();

private:
  OLEDDisplay();                            // Private constructor for singleton
  OLEDDisplay(const OLEDDisplay&) = delete;  // Disable copy
  void operator=(const OLEDDisplay&) = delete;

  U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;
};

#endif // OLED_DISPLAY_H
