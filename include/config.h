#ifndef CONFIG_H
#define CONFIG_H

#include "Common/ConfigData.h"
#include <Arduino.h>

// WiFi Configuration
#define WIFI_SSID "P425"
#define WIFI_PASSWORD "87654321"

// ThingsBoard / Core IOT Configuration
#define THINGSBOARD_SERVER "app.coreiot.io"
#define THINGSBOARD_PORT 1883U
#define ACCESS_TOKEN "oAp8JA4Xj5OUW9UwFrhh"

// OTA metadata for CoreIoT / ThingsBoard firmware updates.
// Keep title stable for this device family and bump version on each release.
#define COREIOT_FW_TITLE "iot_assignment_252"
#define COREIOT_FW_VERSION "1.4.0"

// CoreIoT local alert thresholds used for telemetry publishing.
#define COREIOT_ALERT_TEMP_THRESHOLD 35.0f
#define COREIOT_ALERT_HUMI_THRESHOLD 40.0f

// Environment thresholds used for LED/OLED state indication.
#define ENV_TEMP_WARNING_THRESHOLD 30.0f
#define ENV_TEMP_CRITICAL_THRESHOLD COREIOT_ALERT_TEMP_THRESHOLD
#define ENV_HUMI_WARNING_LOW_THRESHOLD 45.0f
#define ENV_HUMI_CRITICAL_LOW_THRESHOLD COREIOT_ALERT_HUMI_THRESHOLD
#define ENV_HUMI_CRITICAL_HIGH_THRESHOLD 80.0f

// Temperature bands used for LED blink frequency selection.
#define ENV_TEMP_BLINK_LOW_THRESHOLD 24.0f
#define ENV_TEMP_BLINK_HIGH_THRESHOLD ENV_TEMP_WARNING_THRESHOLD

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
#define BUTTON_2_GPIO GPIO_NUM_0
#define IR_RECEIVER_GPIO GPIO_NUM_6

// IR Remote input
#define IR_REMOTE_ENABLE_LEARN_LOG 0
#define IR_REMOTE_DEBOUNCE_MS 250U
#define IR_REMOTE_CODE_MODE_NEXT 0xBA45FF00
#define IR_REMOTE_CODE_FAN_TOGGLE 0xF30CFF00
#define IR_REMOTE_CODE_PUMP_TOGGLE 0xE718FF00
#define IR_REMOTE_CODE_GREEN_LED_TOGGLE 0xA15EFF00
#define IR_REMOTE_CODE_NEO_LED_TOGGLE 0xF708FF00

// DHT
#define DHT11_GPIO GPIO_NUM_1
#define DHT_TYPE DHT11

// #define DHT20_GPIO GPIO_NUM_2
// #define DHT_TYPE DHT20

#define READ_INTERVAL 3000

// Plant-care actuator mapping.
// Fan output is driven by PWM when configured. Pump stays digital on/off.
#define PLANT_CARE_FAN_RELAY_GPIO GPIO_NUM_21
#define PLANT_CARE_PUMP_RELAY_GPIO -1
#define PLANT_CARE_FAN_PWM_CHANNEL 0
#define PLANT_CARE_FAN_PWM_FREQ 25000U
#define PLANT_CARE_FAN_PWM_RESOLUTION 8
#define PLANT_CARE_FAN_PWM_DUTY_ON 255U

// Display (OLED via U8g2)
#define I2C_SDA 11
#define I2C_SCL 12

// Queue sizing
#define QUEUE_SIZE 10

// Global variables (External declarations)
extern volatile float globalTemp;
extern volatile float globalHumi;

extern ConfigData globalConfig;

// FreeRTOS Task Handles
extern TaskHandle_t task_sys_manager_handle;
extern TaskHandle_t task_input_manager_handle;
extern TaskHandle_t task_manager_handle;
extern TaskHandle_t task_normal_mode_handle;
extern TaskHandle_t task_manual_mode_handle;
extern TaskHandle_t task_ap_mode_handle;
extern TaskHandle_t task_accesspoint_mode_handle;
extern TaskHandle_t task_button_handle;
extern TaskHandle_t task_sensor_handle;
extern TaskHandle_t task_ir_remote_handle;
extern TaskHandle_t task_plant_care_handle;

// FreeRTOS Semaphores
extern SemaphoreHandle_t btnSemaphore;
extern SemaphoreHandle_t sensorSemaphore;

#endif // CONFIG_H
