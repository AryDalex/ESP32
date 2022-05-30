#include <stdio.h>
#include "esp_log.h"

const char *TAG = "main.c";

void app_main(void) {
    printf("Hello Rayssa\n");
    ESP_LOGI(TAG, "info");
    ESP_LOGI(TAG, "warning");
    ESP_LOGI(TAG, "error");
} 