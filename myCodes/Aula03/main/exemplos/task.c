#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

const char *TAG = "tasks.c";

void task1(void *param)
{ // ler temperatura
    while (true)
    {
        ESP_LOGI(TAG, "core %d/line %d/%s/reading temperature/%d",
                 xPortGetCoreID(), __LINE__, __func__,
                 uxTaskGetStackHighWaterMark(NULL));
        vTaskDelay(1000 / portTICK_PERIOD_MS); // 1s
    }
    vTaskDelete(NULL);
}

void task2()
{ // ler umidade
    while (true)
    {
        ESP_LOGW(TAG, "core %d/line %d/%s/reading humidity/%d",
                 xPortGetCoreID(), __LINE__, __func__,
                 uxTaskGetStackHighWaterMark(NULL));
        vTaskDelay(2000 / portTICK_PERIOD_MS); // 2s
    }
    vTaskDelete(NULL);
}

void app_main(void)
{
    ESP_LOGI(TAG, "core %d/line %d/%s/starting ", xPortGetCoreID(), __LINE__,
             __func__);

    // nao funciona
    // task1(NULL);
    // task2(NULL);

    // criar uma task
    // xTaskCreate(&endereco, name, tamanho, null, nivel de importancia, null, 
    // nucleo);

    // xTaskCreate(&task1, "temperature reading", 2048, NULL, 2, NULL);
    // xTaskCreate(&task2, "humidity reading", 2048, NULL, 2, NULL);

    // assim podera fazer tarefas paralelamente
    xTaskCreatePinnedToCore(&task1, "temperature reading", 2048, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(&task2, "humidity reading", 2048, NULL, 2, NULL, APP_CPU_NUM);
}