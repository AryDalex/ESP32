#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define PIN_LED_SERIE 23
#define PIN_LED_PARALELO 22
#define PIN_BUTTON 19

int button = 0;

void reading_button_task(void *param)
{
    while (1)
    {
        if (gpio_get_level(PIN_BUTTON))
        {
            button++;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void led_task(void *param)
{
    while (1)
    {
        if (button == 1)
        {
            gpio_set_level(PIN_LED_SERIE, 1);
            gpio_set_level(PIN_LED_PARALELO, 0);
        }
        else if (button == 2)
        {
            gpio_set_level(PIN_LED_SERIE, 0);
            gpio_set_level(PIN_LED_PARALELO, 1);
        } 
        else if (button == 3)
        {
            gpio_set_level(PIN_LED_SERIE, 1);
            gpio_set_level(PIN_LED_PARALELO, 1);
        }
        else if (button == 4)
        {
            gpio_set_level(PIN_LED_SERIE, 0);
            gpio_set_level(PIN_LED_PARALELO, 0);
        }
        else if(button > 4) 
        {
            button = 1;
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    gpio_pad_select_gpio(PIN_BUTTON);
    gpio_set_direction(PIN_BUTTON, GPIO_MODE_INPUT);
    gpio_pulldown_en(PIN_BUTTON);
    gpio_pullup_dis(PIN_BUTTON);

    gpio_pad_select_gpio(PIN_LED_SERIE);
    gpio_set_direction(PIN_LED_SERIE, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(PIN_LED_PARALELO);
    gpio_set_direction(PIN_LED_PARALELO, GPIO_MODE_OUTPUT);

    xTaskCreate(&reading_button_task, "reading button", 2048, NULL, 10, NULL);
    xTaskCreate(&led_task, "leds", 2048, NULL, 9, NULL);
}
