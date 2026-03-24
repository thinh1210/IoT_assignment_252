#include "Input/InputLayer.h"
#include "Input/DHTSensor.h"
#include "esp_log.h"

static const char *TAG = "InputLayer";

// Define static variables
QueueHandle_t *InputLayer::qProcessing = NULL;
ButtonHandler *InputLayer::btnHandler = NULL;
SemaphoreHandle_t InputLayer::btnSemaphore = NULL;
SystemMode InputLayer::currentMode = SystemMode::NORMAL_MODE;

TaskHandle_t InputLayer::task_normal_mode_handle = NULL;
TaskHandle_t InputLayer::task_accesspoint_mode_handle = NULL;

void InputLayer::init(QueueHandle_t *qProc) {
  ESP_LOGI(TAG, "Entering %s", __FUNCTION__);
  qProcessing = qProc;

  // 1. Create counting semaphore to receive button events
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

  // Initialize DHT Sensor
  DHTSensor::init();

  if (qProcessing == NULL || btnSemaphore == NULL) {
    ESP_LOGE(TAG, "Failed to initialize: Queue or Semaphore is NULL");
  } else {
    ESP_LOGI(TAG, "InputLayer initialized successfully.");
  }
}

void InputLayer::initNormalMode() {
  ESP_LOGI(TAG, "Initialize Input Normal Mode Task.");
  xTaskCreate(task_normal_mode, "task_normal_mode", 4096, NULL, 6,
              &task_normal_mode_handle);
}

void InputLayer::initAccessPointMode() {
  ESP_LOGI(TAG, "Initialize Input AccessPoint Mode Task.");
  xTaskCreate(task_accesspoint_mode, "task_accesspoint_mode", 4096, NULL, 6,
              &task_accesspoint_mode_handle);
  if (task_accesspoint_mode_handle != NULL) {
    vTaskSuspend(task_accesspoint_mode_handle);
  }
}

void InputLayer::switchMode(SystemMode newMode) {
  if (currentMode == newMode)
    return;
  ESP_LOGI(TAG, "Input switching Mode: %d -> %d", (int)currentMode,
           (int)newMode);

  switch (newMode) {
  case SystemMode::NORMAL_MODE:
    if (task_accesspoint_mode_handle != NULL)
      vTaskSuspend(task_accesspoint_mode_handle);
    if (task_normal_mode_handle != NULL)
      vTaskResume(task_normal_mode_handle);
    break;
  case SystemMode::ACCESSPOINT_MODE:
    if (task_normal_mode_handle != NULL)
      vTaskSuspend(task_normal_mode_handle);
    if (task_accesspoint_mode_handle != NULL)
      vTaskResume(task_accesspoint_mode_handle);
    break;
  }

  currentMode = newMode;
}

void InputLayer::task_normal_mode(void *param) {
  uint32_t lastSensorRead = 0;
  while (1) {
    uint32_t now = millis();

    // 1. Must call ButtonHandler loop() to update state
    if (btnHandler != NULL) {
      btnHandler->loop();
    }

    // 2. Wait for semaphore event notification (non-blocking)
    if (xSemaphoreTake(btnSemaphore, 0) == pdPASS) {
      handleButtonEvents();
    }

    // 3. Periodic sensor reading for Normal mode (every 3000ms)
    if (now - lastSensorRead >= 3000) {
      readSensors();
      lastSensorRead = now;
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void InputLayer::task_accesspoint_mode(void *param) {
  while (1) {
    if (btnHandler != NULL) {
      btnHandler->loop();
    }

    if (xSemaphoreTake(btnSemaphore, 0) == pdPASS) {
      handleButtonEvents();
    }

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void InputLayer::readSensors() {
  float temp, humi;
  // Only send event if DHT reading is successful
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

// Function to check which button is being pressed (using while loop)
void InputLayer::handleButtonEvents() {
  ButtonEvent lastEvt = ButtonEvent::NONE;
  int btnIdx = -1;

  // REQUIREMENT: A function to read which button is being pressed in a while loop
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

    // Send info to ProcessingLayer.
    SystemEvent event;
    if (lastEvt == ButtonEvent::LONG_PRESS /*&& btnIdx == 0*/) {
      event.type = EventType::MODE_CHANGE;
    } else {
      event.type = EventType::BUTTON_PRESSED;
    }

    if (qProcessing != NULL) {
      xQueueSend(*qProcessing, &event, 0);
    }
  }
}
