#include <Arduino.h>
#include <DHT.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <esp_log.h>

// ─── Cấu hình ngoại vi ─────────────────────────────────────────────────────
#define DHT11_GPIO 1
#define DHT_TYPE DHT11
#define READ_INTERVAL 3000

#define GREEN_LED_GPIO GPIO_NUM_48
#define BUTTON_PIN GPIO_NUM_47

#define I2C_SDA GPIO_NUM_11
#define I2C_SCL GPIO_NUM_12

static const char *TAG = "IoT_U8g2";

// ─── Khởi tạo màn hình U8g2 (SSD1306 128x64, I2C) ──────────────────────────
// F: Full Buffer, R0: Không xoay màn hình
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE,
                                        /* clock=*/I2C_SCL, /* data=*/I2C_SDA);

// ─── Cấu trúc dữ liệu & Biến toàn cục ──────────────────────────────────────
typedef struct {
  float temperature;
  float humidity;
} SensorData_t;

static QueueHandle_t dataQueue = NULL;
static SemaphoreHandle_t syncSemaphore = NULL;
static SemaphoreHandle_t mutexData = NULL;

static SensorData_t latestData = {0, 0};
static int blinkDelay = 1000;
static int displayMode = 0;

DHT dht(DHT11_GPIO, DHT_TYPE);

// ─── Task 1: Đọc cảm biến ──────────────────────────────────────────────────
void task_input(void *pvParameters) {
  dht.begin();
  SensorData_t data;
  while (1) {
    data.temperature = dht.readTemperature();
    data.humidity = dht.readHumidity();

    if (!isnan(data.temperature) && !isnan(data.humidity)) {
      xQueueSend(dataQueue, &data, pdMS_TO_TICKS(100));
    }
    vTaskDelay(pdMS_TO_TICKS(READ_INTERVAL));
  }
}

// ─── Task 2: Xử lý Logic ───────────────────────────────────────────────────
void task_processing(void *pvParameters) {
  SensorData_t received;
  while (1) {
    if (xQueueReceive(dataQueue, &received, portMAX_DELAY) == pdPASS) {
      int newDelay = (received.temperature < 26.0)
                         ? 1000
                         : (received.temperature <= 30.0 ? 500 : 100);

      if (xSemaphoreTake(mutexData, pdMS_TO_TICKS(50)) == pdTRUE) {
        latestData = received;
        blinkDelay = newDelay;
        xSemaphoreGive(mutexData);
      }
      xSemaphoreGive(syncSemaphore);
    }
  }
}

// ─── Task 3: Điều khiển LED ────────────────────────────────────────────────
void task_output_led(void *pvParameters) {
  pinMode(GREEN_LED_GPIO, OUTPUT);
  int localDelay = 1000;
  while (1) {
    if (xSemaphoreTake(syncSemaphore, 0) == pdTRUE) {
      if (xSemaphoreTake(mutexData, pdMS_TO_TICKS(10)) == pdTRUE) {
        localDelay = blinkDelay;
        xSemaphoreGive(mutexData);
      }
    }
    digitalWrite(GREEN_LED_GPIO, HIGH);
    vTaskDelay(pdMS_TO_TICKS(localDelay));
    digitalWrite(GREEN_LED_GPIO, LOW);
    vTaskDelay(pdMS_TO_TICKS(localDelay));
  }
}

// ─── Task 4: Nút bấm (GPIO 47) ─────────────────────────────────────────────
void task_button(void *pvParameters) {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  bool lastState = HIGH;
  while (1) {
    bool currentState = digitalRead(BUTTON_PIN);
    if (lastState == HIGH && currentState == LOW) {
      if (xSemaphoreTake(mutexData, pdMS_TO_TICKS(50)) == pdTRUE) {
        displayMode = (displayMode + 1) % 3;
        xSemaphoreGive(mutexData);
      }
      vTaskDelay(pdMS_TO_TICKS(250)); // Chống rung
    }
    lastState = currentState;
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

// ─── Task 5: Hiển thị OLED (U8g2) ──────────────────────────────────────────
void task_output_oled(void *pvParameters) {
  SensorData_t displayData;
  int currentMode;
  char buf[32];

  while (1) {
    if (xSemaphoreTake(mutexData, pdMS_TO_TICKS(50)) == pdTRUE) {
      displayData = latestData;
      currentMode = displayMode;
      xSemaphoreGive(mutexData);
    }

    u8g2.clearBuffer();   // Xóa bộ đệm
    u8g2.setFontMode(1);  // Nền trong suốt
    u8g2.setDrawColor(1); // Màu trắng

    switch (currentMode) {
    case 0: // Màn hình Tổng quát
      u8g2.setFont(u8g2_font_6x10_tr);
      u8g2.drawStr(30, 10, "SYSTEM MONITOR");
      u8g2.drawHLine(0, 12, 128);

      sprintf(buf, "Temp: %.1f C", displayData.temperature);
      u8g2.drawStr(10, 30, buf);

      sprintf(buf, "Humi: %.1f %%", displayData.humidity);
      u8g2.drawStr(10, 45, buf);

      u8g2.setFont(u8g2_font_4x6_tr);
      sprintf(buf, "Blink delay: %d ms", blinkDelay);
      u8g2.drawStr(10, 60, buf);
      break;

    case 1: // Màn hình Nhiệt độ lớn
      u8g2.setFont(u8g2_font_6x10_tr);
      u8g2.drawStr(0, 10, "TEMPERATURE");
      u8g2.setFont(u8g2_font_logisoso24_tr); // Font số lớn
      sprintf(buf, "%.1f C", displayData.temperature);
      u8g2.drawStr(15, 45, buf);
      u8g2.drawFrame(0, 15, 128, 49);
      break;

    case 2: // Màn hình Độ ẩm lớn
      u8g2.setFont(u8g2_font_6x10_tr);
      u8g2.drawStr(0, 10, "HUMIDITY");
      u8g2.setFont(u8g2_font_logisoso32_tr);
      sprintf(buf, "%d%%", (int)displayData.humidity);
      u8g2.drawStr(25, 50, buf);
      break;
    }

    u8g2.sendBuffer(); // Đẩy dữ liệu từ buffer lên màn hình
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

// ─── Setup ─────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  // Khởi tạo U8g2
  u8g2.begin();

  dataQueue = xQueueCreate(5, sizeof(SensorData_t));
  syncSemaphore = xSemaphoreCreateBinary();
  mutexData = xSemaphoreCreateMutex();

  if (dataQueue && syncSemaphore && mutexData) {
    xTaskCreate(task_input, "Input", 4096, NULL, 5, NULL);
    xTaskCreate(task_processing, "Logic", 4096, NULL, 5, NULL);
    xTaskCreate(task_output_led, "LED", 2048, NULL, 5, NULL);
    xTaskCreate(task_button, "Button", 2048, NULL, 6, NULL);
    xTaskCreate(task_output_oled, "OLED", 4096, NULL, 4, NULL);
  }
}

void loop() { vTaskDelete(NULL); }