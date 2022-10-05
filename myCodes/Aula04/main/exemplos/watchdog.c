#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void task1(void *param)
{
    while (1) {
        vTaskDelay(1); // 1 tick
        vTaskDelay( pdMS_TO_TICKS( 100 ) ); // delay de 100 milissegundos para ticks
        vTaskDelay( 100 / portTICK_PERIOD_MS ); // delay de 100 milissegundos / ticks
    }
}

void app_main(void)
{
    xTaskCreate(&task1, "task1", 2048, NULL, 1, NULL);
}