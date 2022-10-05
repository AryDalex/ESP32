#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h" 
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_timer.h"

#define PIN_LED 23

esp_timer_handle_t blinkTimerHandler; // Blink timer handler.

void blinkTimerCallback(void* arg)
{
    static bool isOn = false;
    isOn = !isOn;
    gpio_set_level(PIN_LED, isOn);
}

void app_main() 
{
    gpio_pad_select_gpio(PIN_LED);
    gpio_set_direction(PIN_LED, GPIO_MODE_OUTPUT);
    esp_err_t ret;

    esp_timer_create_args_t timer_args = {
        .callback = blinkTimerCallback,
        .arg = NULL,
        .name = "blink",
        .dispatch_method = ESP_TIMER_TASK,
    };
    
    esp_timer_create(&timer_args, &blinkTimerHandler); // criando o timer
    esp_timer_start_periodic(blinkTimerHandler, 100 * 1000); // tempo que o timer estora
    
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(3000));
        esp_timer_stop(blinkTimerHandler);
        esp_timer_start_periodic(blinkTimerHandler, 200 * 1000);

        vTaskDelay(pdMS_TO_TICKS(3000));
        esp_timer_stop(blinkTimerHandler);
        esp_timer_start_periodic(blinkTimerHandler, 300 * 1000);
        
        vTaskDelay(pdMS_TO_TICKS(3000));
        esp_timer_stop(blinkTimerHandler);
        esp_timer_start_periodic(blinkTimerHandler, 400 * 1000);
        
        vTaskDelay(pdMS_TO_TICKS(3000));
        esp_timer_stop(blinkTimerHandler);
        esp_timer_start_periodic(blinkTimerHandler, 500 * 1000);
    }
}