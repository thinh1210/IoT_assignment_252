#ifndef DISPLAY_SERVICE_H
#define DISPLAY_SERVICE_H

#include <Arduino.h>

class DisplayService {
public:
  static void init();
  
  // Được FSM gọi ra ngay khi vừa chuyển mode
  static void showNormalMode(float t, float h);
  static void showAPMode(int signalLevel = 1);

  // Hiển thị một trang và block (wait) trong vòng milliseconds
  // Phù hợp sau khi nhấn lưu màn hình loading/success
  static void showSuccessBlocking(uint32_t duration_ms = 5000);
  static void showLoadingStep(int step);
};

#endif
