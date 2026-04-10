#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include "Common/ConfigData.h"

// WiFi Configuration
#define WIFI_SSID "BKIT_LUGIA_CS2"
#define WIFI_PASSWORD "cselabc5c6"

// ThingsBoard / Core IOT Configuration
#define THINGSBOARD_SERVER "app.coreiot.io" // Example server
#define ACCESS_TOKEN "ypoy5tdtcjno0i3mgvpl"

// define accesspoint
#define ACCESSPOINT_SSID "ESP32"
#define ACCESSPOINT_PASS "12345678"
#define ACCESSPOINT_IP {192, 168, 4, 1}
#define ACCESSPOINT_GATEWAY {192, 168, 4, 1}
#define ACCESSPOINT_SUBNET {255, 255, 255, 0}

// GPIO Definitions
#define GREEN_LED_GPIO GPIO_NUM_48
#define RED_LED_GPIO -1
#define NEO_LED_GPIO GPIO_NUM_45
#define BUTTON_GPIO GPIO_NUM_47
#define BUTTON_2_GPIO GPIO_NUM_0

// DHT
#define DHT11_GPIO GPIO_NUM_1
#define DHT_TYPE DHT11
#define READ_INTERVAL 3000

// Display (OLED via U8g2)
#define I2C_SDA 11
#define I2C_SCL 12

// Queue sizing
#define QUEUE_SIZE 10

// Global variables (External declarations)
extern float globalTemp;
extern float globalHumi;

extern ConfigData globalConfig;

// FreeRTOS Task Handles
extern TaskHandle_t task_sys_manager_handle;
extern TaskHandle_t task_input_manager_handle;
extern TaskHandle_t task_normal_mode_handle;
extern TaskHandle_t task_ap_mode_handle;
extern TaskHandle_t task_button_handle;
extern TaskHandle_t task_sensor_handle;

// FreeRTOS Semaphores
extern SemaphoreHandle_t btnSemaphore;
extern SemaphoreHandle_t sensorSemaphore;

#endif // CONFIG_H
