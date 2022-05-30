// 4. O ESP32 possui um sensor hall integrado. Descubra
// como fazer leituras dele e apresente os dados a cada 500ms

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"

void app_main(void) {
    while(1) {
        int sensor = hall_sensor_read();
        
        printf("Valor da leitura do sensor: %d\n", sensor);
        vTaskDelay(pdMS_TO_TICKS(500)); // delay
    }
}