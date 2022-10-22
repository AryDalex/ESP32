// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "driver/gpio.h"
// #include "esp_log.h"

// #define PIN_LED_SERIE 23
// #define PIN_LED_PARALELO 22
// #define PIN_BUTTON 15

// int button = 0;

// void reading_button_task(void *param)
// {
//     while (true)
//     {
//         if (gpio_get_level(PIN_BUTTON))
//         {
//             button++;
//             ESP_LOGW("button", "%d", button);
//         }
//         vTaskDelay(pdMS_TO_TICKS(100));
//     }
//     vTaskDelete(NULL);
// }

// void led_task(void *param)
// {
//     while (true)
//     {
//         if (button == 1)
//         {
//             gpio_set_level(PIN_LED_SERIE, 1);
//             gpio_set_level(PIN_LED_PARALELO, 0);
//             ESP_LOGW("led", "led no circuito em serie");
//         }
//         else if (button == 2)
//         {
//             gpio_set_level(PIN_LED_SERIE, 0);
//             gpio_set_level(PIN_LED_PARALELO, 1);
//             ESP_LOGW("led", "led no circuito em paralelo");
//         }
//         else if (button == 3)
//         {
//             gpio_set_level(PIN_LED_SERIE, 1);
//             gpio_set_level(PIN_LED_PARALELO, 1);
//             ESP_LOGW("led", "leds no circuito em serie e em paralelo");
//         }
//         else if (button == 4)
//         {
//             gpio_set_level(PIN_LED_SERIE, 0);
//             gpio_set_level(PIN_LED_PARALELO, 0);
//             ESP_LOGW("led", "leds desligados");
//         }
//         else if (button > 4)
//         {
//             button = 1;
//         }
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
//     vTaskDelete(NULL);
// }

// void app_main(void)
// {
//     gpio_pad_select_gpio(PIN_BUTTON);
//     gpio_set_direction(PIN_BUTTON, GPIO_MODE_INPUT);
//     gpio_pulldown_en(PIN_BUTTON);
//     gpio_pullup_dis(PIN_BUTTON);
//     ESP_LOGW("button", "button init");

//     gpio_pad_select_gpio(PIN_LED_SERIE);
//     gpio_set_direction(PIN_LED_SERIE, GPIO_MODE_OUTPUT);
//     gpio_pad_select_gpio(PIN_LED_PARALELO);
//     gpio_set_direction(PIN_LED_PARALELO, GPIO_MODE_OUTPUT);
//     ESP_LOGW("led", "leds init");

//     xTaskCreatePinnedToCore(&reading_button_task, "reading_button_task",
//     2048, NULL, 10, NULL, 0); xTaskCreatePinnedToCore(&led_task, "led_task",
//     2048, NULL, 9, NULL, 0); ESP_LOGW("task", "tasks init");
// }

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define PIN_LED_SERIE 18
#define PIN_LED_PARALELO 19

void app_main(void)
{
    gpio_pad_select_gpio(PIN_LED_SERIE);
    gpio_set_direction(PIN_LED_SERIE, GPIO_MODE_OUTPUT);

    gpio_pad_select_gpio(PIN_LED_PARALELO);
    gpio_set_direction(PIN_LED_PARALELO, GPIO_MODE_OUTPUT);

    while (1)
    {
        ESP_LOGW("led", "leds ligados");
        gpio_set_level(PIN_LED_SERIE, 1);
        gpio_set_level(PIN_LED_PARALELO, 1);

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}