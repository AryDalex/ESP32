#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define PIN_LED 23

const char *TAG = "output.c";

void app_main(void) {
    ESP_LOGI(TAG, "Starting!");

    gpio_pad_select_gpio(PIN_LED); // seleciona o pino do led
    gpio_set_direction(PIN_LED, GPIO_MODE_OUTPUT); // define o led como saida

    bool isOn = 0;

    while(true) {
        isOn = !isOn;
        gpio_set_level(PIN_LED, isOn); // coloca o valor de "isOn" no "pin_led"
        vTaskDelay(pdMS_TO_TICKS(1000)); // delay
    }
}