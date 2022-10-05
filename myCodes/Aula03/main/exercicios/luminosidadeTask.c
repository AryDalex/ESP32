// 3. Faça um programa, utilizando tasks, que a medida que o
// botão é pressionado aumente a intensidade do LED ao atingir o
// valor máximo volte a zero. Adicione 5 passos.

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/dac.h"
#include "esp_log.h"

#define PIN_BUTTON 15

int button = 0;

void button_task(char *param){
    while(1){
        if (gpio_get_level(PIN_BUTTON)){                  
            button++;
            vTaskDelay(400 / portTICK_PERIOD_MS);
        }
    }
}

void luminosidade_task(void *param){
    switch (button){
        case 1: 
            dac_output_enable(DAC_CHANNEL_1); // habilita como saida
            dac_output_voltage(DAC_CHANNEL_1, 250); // luminosidade do led
            break;

        case 2: 
            dac_output_enable(DAC_CHANNEL_1); // habilita como saida
            dac_output_voltage(DAC_CHANNEL_1, 235); // luminosidade do led
            break;

        case 3: 
            dac_output_enable(DAC_CHANNEL_1); // habilita como saida
            dac_output_voltage(DAC_CHANNEL_1, 210); // luminosidade do led
            break;

        case 4: 
            dac_output_enable(DAC_CHANNEL_1); // habilita como saida
            dac_output_voltage(DAC_CHANNEL_1, 200); // luminosidade do led
            break;

        case 5: 
            dac_output_enable(DAC_CHANNEL_1); // habilita como saida
            dac_output_voltage(DAC_CHANNEL_1, 190); // luminosidade do led

            button = 0;
            break;
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
}

void app_main(void) {
    gpio_pad_select_gpio(PIN_BUTTON); // seleciona o pino do botao
    gpio_set_direction(PIN_BUTTON, GPIO_MODE_INPUT); // define o botao como entrada

    gpio_pulldown_en(PIN_BUTTON); // liga como pulldown
    gpio_pullup_dis(PIN_BUTTON); // desliga como pullup

    xTaskCreate(&luminosidade_task, "luminosidade", 2048, NULL, 11, NULL);
    xTaskCreate(&button_task, "button", 2048, NULL, 12, NULL);
}