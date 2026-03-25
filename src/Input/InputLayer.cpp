#include "Input/InputLayer.h"
#include "Input/DHTSensor.h"
#include "esp_log.h"

static const char *TAG = "InputLayer";

// Define static variables
QueueHandle_t *InputLayer::qProcessing = NULL;
ButtonHandler *InputLayer::btnHandler = NULL;
SemaphoreHandle_t InputLayer::btnSemaphore = NULL;
SystemMode InputLayer::currentMode = SystemMode::NORMAL_MODE;

TaskHandle_t InputLayer::task_manager_handle = NULL;
TaskHandle_t InputLayer::task_normal_mode_handle = NULL;
TaskHandle_t InputLayer::task_accesspoint_mode_handle = NULL;

void InputLayer::init(QueueHandle_t *qProc) {
  ESP_LOGI(TAG, "General InputLayer Initialization...");
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

  // Initialize Sensors
  DHTSensor::init();

  // 3. Start Persistent Input Manager (Task that handles events)
  xTaskCreate(task_manager, "input_manager", 4096, NULL, 6, &task_manager_handle);

  ESP_LOGI(TAG, "InputLayer General Init successful.");
}

void InputLayer::task_manager(void *param) {
  ESP_LOGI(TAG, "Input Manager Task started.");
  while (1) {
    // 1. Must call ButtonHandler loop() to update state
    if (btnHandler != NULL) {
      btnHandler->loop();
    }

    // 2. Wait for semaphore event notification (non-blocking or short wait)
    if (xSemaphoreTake(btnSemaphore, pdMS_TO_TICKS(10)) == pdPASS) {
      handleButtonEvents();
    }
    
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void InputLayer::initNormalMode() {
  deinitMode(); // Ensure clean state
  ESP_LOGI(TAG, "Initializing Input Normal Mode Tasks...");
  xTaskCreate(task_normal_mode, "in_normal_task", 4096, NULL, 5, &task_normal_mode_handle);
}

void InputLayer::initAccessPointMode() {
  deinitMode(); // Ensure clean state
  ESP_LOGI(TAG, "Initializing Input AccessPoint Mode Tasks...");
  xTaskCreate(task_accesspoint_mode, "in_ap_task", 4096, NULL, 5, &task_accesspoint_mode_handle);
}

void InputLayer::deinitMode() {
  if (task_normal_mode_handle != NULL) {
    vTaskDelete(task_normal_mode_handle);
    task_normal_mode_handle = NULL;
  }
  if (task_accesspoint_mode_handle != NULL) {
    vTaskDelete(task_accesspoint_mode_handle);
    task_accesspoint_mode_handle = NULL;
  }
  ESP_LOGI(TAG, "Mode tasks deleted.");
}

void InputLayer::switchMode(SystemMode newMode) {
  if (currentMode == newMode) return;
  ESP_LOGI(TAG, "Switching Input Mode: %d -> %d", (int)currentMode, (int)newMode);
  
  if (newMode == SystemMode::NORMAL_MODE) {
    initNormalMode();
  } else {
    initAccessPointMode();
  }
  currentMode = newMode;
}

void InputLayer::task_normal_mode(void *param) {
  uint32_t lastSensorRead = 0;
  while (1) {
    uint32_t now = millis();
    // Periodic sensor reading for Normal mode
    if (now - lastSensorRead >= 3000) {
      readSensors();
      lastSensorRead = now;
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void InputLayer::task_accesspoint_mode(void *param) {
  while (1) {
    // In AP mode, maybe we do nothing or something specific
    vTaskDelay(pdMS_TO_TICKS(1000));
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
