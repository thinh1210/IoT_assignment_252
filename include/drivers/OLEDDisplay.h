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
  void drawAPPage(int level, const char *statusText, const char *careText);
  void drawLoadingPage(int step);
  void drawTelemetryPage(float t, float h, const char *modeText,
                         const char *statusText, const char *careText);
  void drawSuccessPage();
  void playWelcomeAnimation(const char *name);

private:
  OLEDDisplay();                            // Private constructor for singleton
  OLEDDisplay(const OLEDDisplay&) = delete;  // Disable copy
  void operator=(const OLEDDisplay&) = delete;

  void drawWelcomeFrame(const char *name, int16_t offsetX, uint8_t accentWidth,
                        bool invertAccent);

  U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;
};

#endif // OLED_DISPLAY_H
