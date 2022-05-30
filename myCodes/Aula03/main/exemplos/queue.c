#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
xQueueHandle queue;

void sender(void *params)
{
    int count = 0;
    while (true)
    {
        count++;

        // mandar a queue
        // xQueueSend(queue, &endereco, tempo)
        if (xQueueSend(queue, &count, 10) != pdTRUE)
        {
            printf("Queue FULL\n");
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS); // tempo de mandar
    }
}

void receiver(void *params)
{
    while (true)
    {
        int rxInt;

        // recebe a queue
        // xQueueReceive(queue, &endereco, tempo)
        if (xQueueReceive(queue, &rxInt, 0) == pdTRUE)
        {
            printf("received %d\n", rxInt);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS); // tempo de receber
    }
}

void app_main(void)
{
    // para criar a queue
    // xQueueCreate(posicoes, tipo)
    queue = xQueueCreate(3, sizeof(int));

    // criar uma task
    // xTaskCreate(&endereco, name, tamanho, null, nivel de importancia, null, 
    // nucleo);
    xTaskCreate(&sender, "send", 2048, NULL, 2, NULL);
    xTaskCreate(&receiver, "receive", 2048, NULL, 1, NULL);
}