#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include "Common/ConfigData.h"

// WiFi Configuration
#define WIFI_SSID "BKIT_LUGIA_CS2"
#define WIFI_PASSWORD "cselabc5c6"

// ThingsBoard / Core IOT Configuration
#define THINGSBOARD_SERVER "app.coreiot.io"
#define THINGSBOARD_PORT 1883U
#define ACCESS_TOKEN "dT4E1leI29Lbq1F46vHo"

// OTA metadata for CoreIoT / ThingsBoard firmware updates.
// Keep title stable for this device family and bump version on each release.
#define COREIOT_FW_TITLE "iot_assignment_252"
#define COREIOT_FW_VERSION "1.0.0"

// CoreIoT local alert thresholds used for telemetry publishing.
#define COREIOT_ALERT_TEMP_THRESHOLD 35.0f
#define COREIOT_ALERT_HUMI_THRESHOLD 40.0f

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

// Plant-care actuator mapping.
// Set these to the relay GPIOs used for the fan/pump. Keep -1 to disable output.
#define PLANT_CARE_FAN_RELAY_GPIO -1
#define PLANT_CARE_PUMP_RELAY_GPIO -1

// Display (OLED via U8g2)
#define I2C_SDA 11
#define I2C_SCL 12

// Queue sizing
#define QUEUE_SIZE 10

// Global variables (External declarations)
extern float globalTemp;
extern float globalHumi;

extern ConfigData globalConfig;

#endif // CONFIG_H
