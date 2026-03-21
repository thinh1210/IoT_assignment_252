#include "Input/InputLayer.h"
#include "Input/DHTSensor.h"
#include "esp_log.h"

static const char *TAG = "InputLayer";

// Define static variables
QueueHandle_t *InputLayer::qProcessing = NULL;
ButtonHandler *InputLayer::btnHandler = NULL;
SemaphoreHandle_t InputLayer::btnSemaphore = NULL;
SystemMode InputLayer::currentMode = SystemMode::MODE_A;

TaskHandle_t InputLayer::modeATaskHandle = NULL;
TaskHandle_t InputLayer::modeBTaskHandle = NULL;

void InputLayer::init(QueueHandle_t *qProc) {
  ESP_LOGI(TAG, "Entering %s", __FUNCTION__);
  qProcessing = qProc;

  // 1. Tạo counting semaphore để đón nhận các sự kiện nhấn nút
  btnSemaphore = xSemaphoreCreateCounting(20, 0);

  // 2. Initialize ButtonHandler
  btnHandler = new ButtonHandler();
  btnHandler->setSemaphore(btnSemaphore);

  if (BUTTON_GPIO != -1) {
    btnHandler->addButton(BUTTON_GPIO, 2000, true);
  }

  if (BUTTON_2_GPIO != -1) {
    btnHandler->addButton(BUTTON_2_GPIO, 2000, true);
  }

  btnHandler->begin();

  if (GREEN_LED_GPIO != -1)
    pinMode(GREEN_LED_GPIO, OUTPUT);

  // Khởi tạo DHT Sensor
  DHTSensor::init();

  if (qProcessing == NULL || btnSemaphore == NULL) {
    ESP_LOGE(TAG, "Failed to initialize: Queue or Semaphore is NULL");
  } else {
    ESP_LOGI(TAG, "InputLayer initialized successfully.");
  }
}

void InputLayer::initModeA() {
  ESP_LOGI(TAG, "Initialize Input Mode A Task.");
  xTaskCreate(taskModeALoop, "InTaskModeA", 4096, NULL, 6, &modeATaskHandle);
}

void InputLayer::initModeB() {
  ESP_LOGI(TAG, "Initialize Input Mode B Task.");
  xTaskCreate(taskModeBLoop, "InTaskModeB", 4096, NULL, 6, &modeBTaskHandle);
  if (modeBTaskHandle != NULL) {
    vTaskSuspend(modeBTaskHandle);
  }
}

void InputLayer::switchMode(SystemMode newMode) {
  if (currentMode == newMode)
    return;
  ESP_LOGI(TAG, "Input switching Mode: %d -> %d", (int)currentMode,
           (int)newMode);

  switch (newMode) {
  case SystemMode::MODE_A:
    if (modeBTaskHandle != NULL)
      vTaskSuspend(modeBTaskHandle);
    if (modeATaskHandle != NULL)
      vTaskResume(modeATaskHandle);
    break;
  case SystemMode::MODE_B:
    if (modeATaskHandle != NULL)
      vTaskSuspend(modeATaskHandle);
    if (modeBTaskHandle != NULL)
      vTaskResume(modeBTaskHandle);
    break;
  }

  currentMode = newMode;
}

void InputLayer::taskModeALoop(void *param) {
  uint32_t lastSensorRead = 0;
  while (1) {
    uint32_t now = millis();

    // 1. Phải gọi loop() của ButtonHandler để cập nhật trạng thái
    if (btnHandler != NULL) {
      btnHandler->loop();
    }

    // 2. Chờ semaphore thông báo sự kiện (non-blocking)
    if (xSemaphoreTake(btnSemaphore, 0) == pdPASS) {
      handleButtonEvents();
    }

    // 3. Periodic sensor reading cho Mode A (chậm: 3000ms)
    if (now - lastSensorRead >= 3000) {
      readSensors();
      lastSensorRead = now;
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void InputLayer::taskModeBLoop(void *param) {
  uint32_t lastSensorRead = 0;
  while (1) {
    uint32_t now = millis();
    // 1. Phải gọi loop() của ButtonHandler để cập nhật trạng thái
    if (btnHandler != NULL) {
      btnHandler->loop();
    }

    // 2. Chờ semaphore thông báo sự kiện (non-blocking)
    if (xSemaphoreTake(btnSemaphore, 0) == pdPASS) {
      handleButtonEvents();
    }

    // 3. Periodic sensor reading cho Mode B (nhanh: 1000ms)
    if (now - lastSensorRead >= 1000) {
      readSensors();
      lastSensorRead = now;
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void InputLayer::readSensors() {
  float temp, humi;
  // Chỉ gửi sự kiện nếu đọc DHT thành công
  if (DHTSensor::readData(temp, humi)) {
    globalTemp = temp;
    globalHumi = humi;

    SystemEvent event;
    event.type = EventType::SENSOR_DATA_READY;
    if (qProcessing != NULL) {
      xQueueSend(*qProcessing, &event, 0);
    }
  }
}

// Function để đọc ra xem nút nào đang được nhấn (sử dụng vòng while)
void InputLayer::handleButtonEvents() {
  ButtonEvent lastEvt = ButtonEvent::NONE;
  int btnIdx = -1;

  // THEO YÊU CẦU: Một function đọc ra xem nút nào đang nhấn trong vòng while
  while ((btnIdx = btnHandler->getNextEvent(lastEvt)) != -1) {

    switch (lastEvt) {
    case ButtonEvent::SINGLE_CLICK:
      ESP_LOGI(TAG, "BTN %d: SINGLE CLICK (RELEASED)", btnIdx);
      break;
    case ButtonEvent::DOUBLE_CLICK:
      ESP_LOGI(TAG, "BTN %d: DOUBLE CLICK (RELEASED)", btnIdx);
      break;
    case ButtonEvent::TRIPLE_CLICK:
      ESP_LOGI(TAG, "BTN %d: TRIPLE CLICK (RELEASED)", btnIdx);
      break;
    case ButtonEvent::LONG_PRESS:
      ESP_LOGI(TAG, "BTN %d: LONG PRESS (RELEASED)", btnIdx);
      break;
    default:
      break;
    }

    // Gửi thông tin sang ProcessingLayer.
    SystemEvent event;
    if (lastEvt == ButtonEvent::LONG_PRESS && btnIdx == 0) {
      event.type = EventType::MODE_CHANGE;
    } else {
      event.type = EventType::BUTTON_PRESSED;
    }

    if (qProcessing != NULL) {
      xQueueSend(*qProcessing, &event, 0);
    }
  }
}
