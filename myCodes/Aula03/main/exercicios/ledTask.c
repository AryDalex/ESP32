// 2. Faça um programa, utilizando tasks, que ao pressionar o
// botão o led pisca e pressionar novamente o led fica ligado

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define PIN_BUTTON 15
#define PIN_LED 23

const char *TAG = "tasks.c";

bool button = 0;

void button_task(char *param){
    while(1){
        if (gpio_get_level(PIN_BUTTON) == 1){               
            while(gpio_get_level(PIN_BUTTON) == 1); // evita mal contato      
            button = !button;
        }
        for (long i = 0; i < 1000000; i++) {} // delay
    }
}

void led_task(char *param){
    while (1) {
        switch (button) {
            case 0: // 10Hz // led piscando
                gpio_set_level(PIN_LED, 1); // led ligado
                vTaskDelay(pdMS_TO_TICKS(50)); // delay
                gpio_set_level(PIN_LED, 0); // led desligado
                vTaskDelay(pdMS_TO_TICKS(50)); // delay
                break;

            case 1: // led ligado
                gpio_set_level(PIN_LED, 1); // led ligado
                break;
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void app_main(void) {
    gpio_pad_select_gpio(PIN_LED); // seleciona o pino do led
    gpio_set_direction(PIN_LED, GPIO_MODE_OUTPUT); // define o led como saida
    
    gpio_pad_select_gpio(PIN_BUTTON); // seleciona o pino do botao
    gpio_set_direction(PIN_BUTTON, GPIO_MODE_INPUT); // define o botao como entrada

    gpio_pulldown_en(PIN_BUTTON); // liga como pulldown
    gpio_pullup_dis(PIN_BUTTON); // desliga como pullup

    xTaskCreatePinnedToCore(&led_task, "led pisca", 2048, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(&button_task, "button", 2048, NULL, 2, NULL, 0);
}