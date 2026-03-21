#include "Input/ButtonHandler.h"
#include "esp_log.h"

static const char *TAG = "ButtonHandler";

ButtonHandler::ButtonHandler() {
    _eventSemaphore = NULL;
}

void ButtonHandler::addButton(uint8_t pin, uint32_t longPressMs, bool activeLow) {
    _buttons.emplace_back(pin, longPressMs, activeLow);
    ESP_LOGI(TAG, "Button added on GPIO %d", pin);
}

void ButtonHandler::setSemaphore(SemaphoreHandle_t semaphore) {
    _eventSemaphore = semaphore;
}

void ButtonHandler::begin() {
    for(auto &btn : _buttons) {
        if (btn.activeLow) {
            pinMode(btn.pin, INPUT_PULLUP);
        } else {
            pinMode(btn.pin, INPUT_PULLDOWN); 
        }
        ESP_LOGI(TAG, "Pin %d initialized (Active %s)", btn.pin, btn.activeLow ? "LOW" : "HIGH");
    }
}

void ButtonHandler::loop() {
    for (int i = 0; i < (int)_buttons.size(); i++) {
        ButtonState &btn = _buttons[i];

        // 1. Đọc trạng thái vật lý
        int reading = digitalRead(btn.pin);
        bool isCurrentlyPressedRaw = (reading == (btn.activeLow ? LOW : HIGH));

        // 2. Xử lý chống rung
        if (reading != btn.lastReading) {
            btn.lastDebounceTime = millis();
        }
        btn.lastReading = reading;

        if ((millis() - btn.lastDebounceTime) > btn.debounceTimeMs) {
            
            // XỬ LÝ CHUYỂN TRẠNG THÁI (Edge Detection)
            if (isCurrentlyPressedRaw != btn.currentState) {
                btn.currentState = isCurrentlyPressedRaw;

                // --- SỰ KIỆN NHẤN XUỐNG ---
                if (btn.currentState == true) {
                    btn.pressStartTime = millis();
                    btn.isLongPressing = false;
                } 
                // --- SỰ KIỆN NHẢ RA (RELEASE) ---
                else {
                    uint32_t duration = millis() - btn.pressStartTime;

                    // Nếu nhả ra sau khi đã nhấn lâu (Long Press)
                    if (duration >= btn.longPressTimeMs) {
                        btn.lastEvent = ButtonEvent::LONG_PRESS;
                        btn.clickCount = 0;
                        btn.pendingClickProcess = false;
                        if (_eventSemaphore != NULL) xSemaphoreGive(_eventSemaphore);
                    } 
                    // Nếu nhả ra bình thường (Check Click Đơn/Đôi/Ba)
                    else if (duration > 50) { // Duration tối thiểu để tính là một click (chống nhiễu thêm)
                        btn.clickCount++;
                        btn.lastReleaseTime = millis();
                        btn.pendingClickProcess = true;
                        
                        // Nếu đã đạt 3 click thì không cần đợi lâu nữa, báo luôn (hoặc có thể tăng giới hạn)
                        if (btn.clickCount >= 3) {
                            btn.lastEvent = ButtonEvent::TRIPLE_CLICK;
                            btn.clickCount = 0;
                            btn.pendingClickProcess = false;
                            if (_eventSemaphore != NULL) xSemaphoreGive(_eventSemaphore);
                        }
                    }
                }
            }
        }

        // 3. XỬ LÝ TIME-OUT ĐỂ XÁC ĐỊNH MỘT LẦN NHẤN HAY NHIỀU LẦN
        // Nếu đang idle (nhả ra) và có click đang chờ xử lý
        if (btn.pendingClickProcess && !btn.currentState) {
            if ((millis() - btn.lastReleaseTime) > btn.clickTimeoutMs) {
                if (btn.clickCount == 1) {
                    btn.lastEvent = ButtonEvent::SINGLE_CLICK;
                } else if (btn.clickCount == 2) {
                    btn.lastEvent = ButtonEvent::DOUBLE_CLICK;
                }
                
                // Gửi thông báo
                btn.clickCount = 0;
                btn.pendingClickProcess = false;
                if (_eventSemaphore != NULL) xSemaphoreGive(_eventSemaphore);
            }
        }
    }
}

int ButtonHandler::getNextEvent(ButtonEvent &event) {
    for (int i = 0; i < (int)_buttons.size(); i++) {
        if (_buttons[i].lastEvent != ButtonEvent::NONE) {
            event = _buttons[i].lastEvent;
            _buttons[i].lastEvent = ButtonEvent::NONE; // Clear flag sau khi đọc
            return i;
        }
    }
    event = ButtonEvent::NONE;
    return -1;
}

bool ButtonHandler::isPressed(int index) {
    if (index < 0 || index >= (int)_buttons.size()) return false;
    return _buttons[index].currentState;
}