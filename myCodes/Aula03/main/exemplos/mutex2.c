#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "freertos/semphr.h"
xSemaphoreHandle mutexBus;

const char *TAG = "mutex.c";

void displayMessage(char *message)
{
    for (int i = 0; i < strlen(message); i++)
    {
        printf("%c", message[i]);
        for (long i = 0; i < 1000000; i++) {}
    }
    printf("\n");
}

void task1(void *param)
{
    while (true)
    {
        if (xSemaphoreTake(mutexBus, 1000))
        {
            displayMessage("temperature is 25c\0");
            xSemaphoreGive(mutexBus);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void task2(void *param)
{
    while (true)
    {
        if (xSemaphoreTake(mutexBus, 1000))
        {
            displayMessage("humidity is 50\0");
            xSemaphoreGive(mutexBus);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void app_main(void)
{
    ESP_LOGI(TAG, "core %d/line %d/%s/starting ", xPortGetCoreID(), __LINE__,
             __func__);
    mutexBus = xSemaphoreCreateMutex();
    xTaskCreatePinnedToCore(&task1, "temperature reading", 2048, NULL, 2, NULL,
                            0);
    xTaskCreatePinnedToCore(&task2, "humidity reading", 2048, NULL, 2, NULL, 1);
}