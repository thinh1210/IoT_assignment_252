// #include <Arduino.h>
// #include <esp_log.h>

// void TaskLEDControl(void *pvParameters) {
//   pinMode(GPIO_NUM_48, OUTPUT); // Initialize LED pin
//   int ledState = 0;
//   while (1) {

//     if (ledState == 0) {
//       digitalWrite(GPIO_NUM_48, HIGH); // Turn ON LED
//     } else {
//       digitalWrite(GPIO_NUM_48, LOW); // Turn OFF LED
//     }
//     ledState = 1 - ledState;
//     vTaskDelay(100);
//   }
// }
// static const char *TAG = "Module";

// void TaskPrint(void *pvParameters) {
//   while (1) {
//     ESP_LOGI(TAG, "This is an info message");
//     vTaskDelay(2000);
//   }
// }

// void setup() {
//   // put your setup code here, to run once:
//   Serial.begin(115200);
//   xTaskCreate(TaskLEDControl, "LED Control", 2048, NULL, 2, NULL);
//   xTaskCreate(TaskPrint, "Print log", 2048, NULL, 1, NULL);
// }

// void loop() {
//   // Serial.println("Hello Custom Board");
//   // delay(1000);
// }


#include <Arduino.h>

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"

#define BOOT_BUTTON_GPIO   GPIO_NUM_0    // ESP32-S3 default BOOT button
#define LED_GPIO           GPIO_NUM_48   // LED on GPIO48

#define SEMAPHORE_MAX_COUNT 10

SemaphoreHandle_t xCountingSemaphore;

void task_monitor_button(void *pvParameters)
{
    while (1) {
        // Active low button
        if (gpio_get_level(BOOT_BUTTON_GPIO) == 0) {
            // Simple debounce
            vTaskDelay(pdMS_TO_TICKS(50));
            if (gpio_get_level(BOOT_BUTTON_GPIO) == 0) {
                // Give the semaphore on each button press (up to max count)
                xSemaphoreGive(xCountingSemaphore);
                // Wait for button release to avoid multiple gives for 1 press
                while (gpio_get_level(BOOT_BUTTON_GPIO) == 0) {
                    vTaskDelay(pdMS_TO_TICKS(10));
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void task_blink_led(void *pvParameters)
{
    while (1) {
        // Wait indefinitely for the semaphore to be available
        if (xSemaphoreTake(xCountingSemaphore, portMAX_DELAY)) {
            // Blink LED once (or multiple times as needed)
            gpio_set_level(LED_GPIO, 1);
            vTaskDelay(pdMS_TO_TICKS(200));
            gpio_set_level(LED_GPIO, 0);
            vTaskDelay(pdMS_TO_TICKS(200));
        }
    }
}

void setup(void)
{
    pinMode(BOOT_BUTTON_GPIO, INPUT_PULLUP);
    pinMode(LED_GPIO, OUTPUT);

    // Create counting semaphore with initial count 0, max count SEMAPHORE_MAX_COUNT
    xCountingSemaphore = xSemaphoreCreateCounting(SEMAPHORE_MAX_COUNT, 0);

    // Create tasks
    xTaskCreate(task_monitor_button, "MonitorButton", 2048, NULL, 10, NULL);
    xTaskCreate(task_blink_led,     "BlinkLED",      2048, NULL,  5, NULL);
}

void loop() {

}