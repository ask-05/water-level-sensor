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

// Tag
static const char *TAG = "WATER_LEVEL_SENSOR";

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

                // Handle blind zone and max range boundaries safely
                if (distance_cm >= 20 && distance_cm <= 450) {
                    ESP_LOGI(TAG, "Distance is %d cm", distance_cm);
                } else {
                    ESP_LOGW(TAG, "Distance %d cm out of reliable bounds (20cm - 450cm)", distance_cm);
                }
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

    xTaskCreate(
        checkWaterDistance,
        "Check Water Level",
        2048,
        NULL,
        5,
        NULL
    );
}
