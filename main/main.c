#include <stdio.h>
#include <string.h>
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "hal/gpio_types.h"
#include "esp_rom_sys.h"
#include "portmacro.h"

// Tag
static const char *TAG = "WATER_LEVEL_SENSOR";

// Global Queue Handle and definitions
static QueueHandle_t distance_queue;

#define QUEUE_LENGTH 20


// Pin Definitions
#define trigPin 5
#define echoPin 18

void checkWaterDistance(void* pvParameters) {
    while(1) {
        // Ensuring trigger is LOW before starting
        gpio_set_level(trigPin, 0);
        esp_rom_delay_us(2);

        // Set trigger HIGH for 10 us to start process
        gpio_set_level(trigPin, 1);
        esp_rom_delay_us(10);
        gpio_set_level(trigPin, 0);

        int64_t us_since_start = esp_timer_get_time();
        int64_t timeout_us = 30000; // 30ms timeout (covers ~5 meters max distance)
        bool timeout_flag = false;

        // Wait for echo pin to go HIGH
        while(gpio_get_level(echoPin) == 0) {
            if (esp_timer_get_time() - us_since_start > timeout_us) {
                timeout_flag = true;
                break;
            }
        }

        if(!timeout_flag) {
            int64_t echo_start = esp_timer_get_time();

            // Wait for echo to go LOW
            while(gpio_get_level(echoPin) == 1) {
                if (esp_timer_get_time() - echo_start > timeout_us) {
                    timeout_flag = true;
                    break;
                }
            }

            if(!timeout_flag) {
                // Calculate distance
                int64_t echoTime = esp_timer_get_time() - echo_start;
                int distance_cm = echoTime / 58;

                // Send data to Queue
                xQueueSend(distance_queue, &distance_cm, pdMS_TO_TICKS(100));
            }
        } 
        
        if (timeout_flag) {
            ESP_LOGE(TAG, "Sensor timeout: No object detected or check wiring.");
            // Give the system a brief moment to recover before the main delay
            vTaskDelay(pdMS_TO_TICKS(10)); 
        }

        // Run again after 500ms
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void handleDistance(void* pvParameters) {
    int distance_cm = 0;
    while(1) {
        if (xQueueReceive(distance_queue, &distance_cm, portMAX_DELAY) == pdTRUE) {
            ESP_LOGI(TAG, "Distance: %dcm", distance_cm);
        }
        
    }
}

void app_main(void)
{
    // Configure Trigger Pin
    gpio_config_t trigConfig = {
        .pin_bit_mask = (1ULL << trigPin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&trigConfig);

    // Configure Echo Pin
    gpio_config_t echoConfig = {
        .pin_bit_mask = (1ULL << echoPin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&echoConfig);

    // Create Queue
    distance_queue = xQueueCreate(QUEUE_LENGTH, sizeof(int));
    if(distance_queue == NULL) {
        ESP_LOGE(TAG, "Could not create queue!");
        return;
    }

    xTaskCreate(
        checkWaterDistance,
        "Check Water Level",
        2048,
        NULL,
        5,
        NULL
    );

    xTaskCreate(
        handleDistance,
        "Handle Distance",
        2048,
        NULL,
        5,
        NULL
    );
}
