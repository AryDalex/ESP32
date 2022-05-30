// 1. Faça um programa, utilizando tasks, que ao manter o botão
// pressionado o LED pisca em uma frequência de 10 Hz e quando
// solto em 2 Hz

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/semphr.h"
xSemaphoreHandle mutexBus;

#define PIN_BUTTON 15
#define PIN_LED 23

const char *TAG = "tasks.c";

void displaySinal(char *sinal) {
    for (int i = 0; i < strlen(sinal); i++) {
        int valueButton = gpio_get_level(PIN_BUTTON); // pega o valor de "pin_switch" e coloca em "level"
        for (long i = 0; i < 1000000; i++) {}
    }
    printf("\n");
}

void task1(void *param) {
    while (true) {
        if (xSemaphoreTake(mutexBus, 1000)) { // 2Hz - botão precionado
            gpio_set_level(PIN_LED, 1); // led ligado
            vTaskDelay(pdMS_TO_TICKS(250)); // delay
            gpio_set_level(PIN_LED, 0); // led desligado
            vTaskDelay(pdMS_TO_TICKS(250)); // delay
        } else if (xSemaphoreTake(mutexBus, 1000)) { // 10Hz - botão solto
            gpio_set_level(PIN_LED, 1); // led ligado
            vTaskDelay(pdMS_TO_TICKS(50)); // delay
            gpio_set_level(PIN_LED, 0); // led desligado
            vTaskDelay(pdMS_TO_TICKS(50)); // delay
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

    mutexBus = xSemaphoreCreateMutex();

    xTaskCreatePinnedToCore(&task1, "led pisca", 2048, NULL, 2, NULL,0);
}

// int receiveButton = 0;

// void button(void *param) {
//     while(1) {
//         int valueButton = gpio_get_level(PIN_BUTTON);

//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
//     vTaskDelete(NULL);
// }

// void led(void *param) {
//     while(true) {
//         if(receiveButton == 1) {
//             gpio_set_level(PIN_LED, 1); // led ligado
//             vTaskDelay(pdMS_TO_TICKS(250)); // delay
//             gpio_set_level(PIN_LED, 0); // led desligado
//             vTaskDelay(pdMS_TO_TICKS(250)); // delay
//             xSemaphoreGive(valueButton);
//         } else if(receiveButton == 0) {
//             gpio_set_level(PIN_LED, 1); // led ligado
//             vTaskDelay(pdMS_TO_TICKS(50)); // delay
//             gpio_set_level(PIN_LED, 0); // led desligado
//             vTaskDelay(pdMS_TO_TICKS(50)); // delay
//             xSemaphoreGive(valueButton);
//         }
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
//     vTaskDelete(NULL);
// }

// void app_main(void) {
//     gpio_pad_select_gpio(PIN_LED); // seleciona o pino do led
//     gpio_set_direction(PIN_LED, GPIO_MODE_OUTPUT); // define o led como saida
//     gpio_pad_select_gpio(PIN_BUTTON); // seleciona o pino do botao
//     gpio_set_direction(PIN_BUTTON, GPIO_MODE_INPUT); // define o botao como entrada

//     gpio_pulldown_en(PIN_BUTTON); // liga como pulldown
//     gpio_pullup_dis(PIN_BUTTON); // desliga como pullup

//     xTaskCreate(&button, "button", 2048, NULL, 2, NULL);
//     xTaskCreate(&led, "led pista", 2048, NULL, 2, NULL);
// }