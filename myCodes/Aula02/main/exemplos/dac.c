#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/dac.h"

void app_main()
{
    dac_output_enable(DAC_CHANNEL_1); // habilita como saida
    dac_output_voltage(DAC_CHANNEL_1, 150); // luminosidade do led
}