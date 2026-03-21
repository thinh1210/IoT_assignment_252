#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

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

#endif // CONFIG_H
