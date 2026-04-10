#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <vector>

// Định nghĩa các loại sự kiện nút nhấn
enum class ButtonEvent {
  NONE,
  SINGLE_CLICK,
  DOUBLE_CLICK,
  TRIPLE_CLICK,
  LONG_PRESS
};

struct ButtonState {
  uint8_t pin;
  bool activeLow;
  uint32_t longPressTimeMs;
  uint32_t debounceTimeMs;
  uint32_t clickTimeoutMs; // Thời gian chờ để xác định là click đơn/đôi/ba (mặc
                           // định 300ms)

  // Biến trạng thái
  bool lastReading;
  bool currentState;
  uint32_t lastDebounceTime;
  uint32_t pressStartTime;
  uint32_t lastReleaseTime;

  // Bộ đếm click
  uint8_t clickCount;
  bool pendingClickProcess;
  bool isLongPressing;

  // Hàng đợi sự kiện (Mỗi button có thể có sự kiện riêng)
  ButtonEvent lastEvent;

  ButtonState(uint8_t p, uint32_t lpMs, bool al)
      : pin(p), activeLow(al), longPressTimeMs(lpMs), debounceTimeMs(50),
        clickTimeoutMs(200), lastReading(al ? HIGH : LOW), currentState(false),
        lastDebounceTime(0), pressStartTime(0), lastReleaseTime(0),
        clickCount(0), pendingClickProcess(false), isLongPressing(false),
        lastEvent(ButtonEvent::NONE) {}
};

class ButtonHandler {
private:
  std::vector<ButtonState> _buttons;
  SemaphoreHandle_t _eventSemaphore;

public:
  ButtonHandler();
  void addButton(uint8_t pin, uint32_t longPressMs = 2000,
                 bool activeLow = true);
  void setSemaphore(SemaphoreHandle_t semaphore);
  void begin();
  void loop();

  // Hàm đọc sự kiện: Trả về index của nút có sự kiện, và loại sự kiện vào tham
  // số event
  int getNextEvent(ButtonEvent &event);

  bool isPressed(int index);
};

#endif