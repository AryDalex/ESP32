#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"

const char *TAG = "timer.c";
esp_timer_handle_t timerHandler;


void timerCallback (void* param)
{
    static int count = 0;
    count++;
    ESP_LOGW(TAG, "timeout %d", count);
}
void app_main()
{
    esp_timer_create_args_t timer_args = {
        .callback = timerCallback,
        .arg = NULL,
        .name = "timer",
        .dispatch_method = ESP_TIMER_TASK,
    };
    
    esp_timer_create(&timer_args, &timerHandler);

    //esp_err_t ret = esp_timer_start_once(timerHandler, 1000 * 1000);
    esp_err_t ret = esp_timer_start_periodic(timerHandler, 1000 * 1000); // timer rodando periodicamente, 1000 milisegundos * 1000 milisegundos = 1s (1000000 milisegundos)

    ESP_LOGI("TAG", "%d start: %s", __LINE__, esp_err_to_name(ret));

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}