// 3. Faça um programa que a medida que o botão é
// pressionado aumente a intensidade do LED ao atingir o valor
// máximo volte a zero. Adicione 5 passos.

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/dac.h"
#include "esp_log.h"

#define PIN_BUTTON 15

void app_main(void) {
    gpio_pad_select_gpio(PIN_BUTTON); // seleciona o pino do botao
    gpio_set_direction(PIN_BUTTON, GPIO_MODE_INPUT); // define o botao como entrada

    gpio_pulldown_en(PIN_BUTTON); // liga como pulldown
    gpio_pullup_dis(PIN_BUTTON); // desliga como pullup

    int button = 0;

    while (1) {
        if(gpio_get_level(PIN_BUTTON)) {
            button += 1;
            vTaskDelay(pdMS_TO_TICKS(500)); // delay
        }

        switch (button) {
            case 1: 
                dac_output_enable(DAC_CHANNEL_1); // habilita como saida
                dac_output_voltage(DAC_CHANNEL_1, 150); // luminosidade do led
                break;

            case 2: 
                dac_output_enable(DAC_CHANNEL_1); // habilita como saida
                dac_output_voltage(DAC_CHANNEL_1, 145); // luminosidade do led
                break;

            case 3: 
                dac_output_enable(DAC_CHANNEL_1); // habilita como saida
                dac_output_voltage(DAC_CHANNEL_1, 140); // luminosidade do led
                break;

            case 4: 
                dac_output_enable(DAC_CHANNEL_1); // habilita como saida
                dac_output_voltage(DAC_CHANNEL_1, 135); // luminosidade do led
                break;

            case 5: 
                dac_output_enable(DAC_CHANNEL_1); // habilita como saida
                dac_output_voltage(DAC_CHANNEL_1, 130); // luminosidade do led

                button = 0;
                break;
        }
    }
}