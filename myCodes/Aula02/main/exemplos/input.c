#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define PIN_SWITCH 15
#define PIN_LED 23

const char *TAG = "input.c";

void app_main(void) {
    gpio_pad_select_gpio(PIN_LED); // seleciona o pino do led
    gpio_set_direction(PIN_LED, GPIO_MODE_OUTPUT); // define o led como saida
    gpio_pad_select_gpio(PIN_SWITCH); // seleciona o pino do botao
    gpio_set_direction(PIN_SWITCH, GPIO_MODE_INPUT); // define o botao como entrada

    gpio_pulldown_en(PIN_SWITCH); // liga como pulldown
    gpio_pullup_dis(PIN_SWITCH); // desliga como pullup

    while(true) {
        int level = gpio_get_level(PIN_SWITCH); // pega o valor de "pin_switch" e coloca em "level"

        gpio_set_level(PIN_LED, level); // coloca o valor de "level" no "pin_led"
        vTaskDelay(1); // delay
    }
}