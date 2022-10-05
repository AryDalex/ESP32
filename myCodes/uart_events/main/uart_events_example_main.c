#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "driver/gpio.h"

static const char *TAG = "uart_events";

#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_16)
#define PIN_BUTTON 15

#define PATTERN_CHR_NUM (3)

#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)

//xQueueHandle verificar;
// xQueueHandle transmitir;

static QueueHandle_t uart0_queue;

int dado;
int verifica;

#define stx 2
#define etx 3

bool flag = 1;

uint8_t data[14];
uint8_t hexBlock1,hexBlock2,hexBlock3,hexBlock4,hexBlock5;
uint8_t hexCalculatedChecksum,hexChecksum;

uint8_t AsciiCharToNum(uint8_t data)
{
    data -= '0';
   
    if (data > 9)
        data -= 7;

    return data;
}

uint8_t verificador(uint8_t *data, uint8_t tamanho)
{    
    uint64_t codigo_decimal = 0;
    uint8_t ver = 0;

printf("Bytes: %d\n", tamanho);

        if (tamanho > 13)
        {
                 for (int x = 1; x < 11; x++){
                     if(data[x] == '0'){
                         ver++;
                     }
                    
                }

                if(ver >= 6){
                    printf("Gasparzinho\n");
                   return 0;
                }
            
            if (data[0] == stx && data[13] == etx)
            {
                hexBlock1 = AsciiCharToNum(data[1]) * 16 + AsciiCharToNum(data[2]);
                hexBlock2 = AsciiCharToNum(data[3]) * 16 + AsciiCharToNum(data[4]);
                hexBlock3 = AsciiCharToNum(data[5]) * 16 + AsciiCharToNum(data[6]);
                hexBlock4 = AsciiCharToNum(data[7]) * 16 + AsciiCharToNum(data[8]);
                hexBlock5 = AsciiCharToNum(data[9]) * 16 + AsciiCharToNum(data[10]);

                codigo_decimal = hexBlock1;
                codigo_decimal = codigo_decimal << 8;

                codigo_decimal |= hexBlock2;
                codigo_decimal = codigo_decimal << 8; 

                codigo_decimal |= hexBlock3;
                codigo_decimal = codigo_decimal << 8;

                codigo_decimal |= hexBlock4;
                codigo_decimal = codigo_decimal << 8;

                codigo_decimal |= hexBlock5;


                // printf("\nCaracteres STX e ETX corretamente recebidos\n");
                printf("ID: ");
                
                printf("%lld\n", codigo_decimal);
                printf("Checksum: ");
               
                printf("%d",data[11]);
                printf("%d",data[12]);

                
                hexChecksum =
                    AsciiCharToNum(data[11]) * 16 + AsciiCharToNum(data[12]);

                // XOR algorithm to calculate checksum of ID blocks.
                hexCalculatedChecksum = hexBlock1 ^ hexBlock2 ^ hexBlock3 ^ hexBlock4 ^ hexBlock5;

                if (hexCalculatedChecksum == hexChecksum)
                {
                    printf("\nChecksum calculado confere com checksum transmitido.\n");
                        return 1;
                }
                else
                {
                    printf("Checksum calculado NAO confere com ""checksum transmitido. Dados corrompidos!");
                }
            }
        }
        printf("\ntag incorreta\n");
        return 0;
    }

static void leitura_button_task(){

    while(1){
        
        if( gpio_get_level(PIN_BUTTON) == 1){
            flag = 1;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    
    }
}    


static void uart_event_task(void *pvParameters)
{
    uart_event_t event;
    size_t buffered_size;
    uint8_t *dtmp = (uint8_t *)malloc(RD_BUF_SIZE);
    static const char *RX_TASK_TAG = "RX_TASK";
    for (;;)
    {

        if (xQueueReceive(uart0_queue, (void *)&event,
                          (portTickType)portMAX_DELAY))
        {
            bzero(dtmp, RD_BUF_SIZE);
            switch (event.type)
            {

            case UART_DATA:

            
                ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
                uart_read_bytes(UART_NUM_2, dtmp, event.size, portMAX_DELAY);

            if(flag == 1){
                ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, dtmp, event.size, ESP_LOG_INFO);

                if(verificador(dtmp, event.size)){
                   ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, dtmp, event.size, ESP_LOG_INFO);
                }
                flag = 0;
            }

                break;

            case UART_FIFO_OVF:
                ESP_LOGI(TAG, "hw fifo overflow");

                uart_flush_input(UART_NUM_2);
                xQueueReset(uart0_queue);
                break;
            
            case UART_BUFFER_FULL:
                ESP_LOGI(TAG, "ring buffer full");

                uart_flush_input(UART_NUM_2);
                xQueueReset(uart0_queue);
                break;
            
            case UART_BREAK:
                ESP_LOGI(TAG, "uart rx break");
                break;
            
            case UART_PARITY_ERR:
                ESP_LOGI(TAG, "uart parity error");
                break;
           
            case UART_FRAME_ERR:
                ESP_LOGI(TAG, "uart frame error");
                break;

            case UART_PATTERN_DET:
                uart_get_buffered_data_len(UART_NUM_2, &buffered_size);
                int pos = uart_pattern_pop_pos(UART_NUM_2);
                ESP_LOGI(TAG,
                         "[UART PATTERN DETECTED] pos: %d, buffered size: %d",
                         pos, buffered_size);
                if (pos == -1)
                {

                    uart_flush_input(UART_NUM_2);
                }
                else
                {
                    uart_read_bytes(UART_NUM_2, dtmp, pos,
                                    100 / portTICK_PERIOD_MS);
                    uint8_t pat[PATTERN_CHR_NUM + 1];
                    memset(pat, 0, sizeof(pat));
                    uart_read_bytes(UART_NUM_2, pat, PATTERN_CHR_NUM,
                                    100 / portTICK_PERIOD_MS);
                    ESP_LOGI(TAG, "read data: %s", dtmp);
                    ESP_LOGI(TAG, "read pat : %s", pat);
                }
                break;
    
            default:
                ESP_LOGI(TAG, "uart event type: %d", event.type);
                break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}


void app_main(void)
{
    esp_log_level_set(TAG, ESP_LOG_INFO);
    // verifica = xQueueCreate(3, sizeof(int));

    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    // Install UART driver, and get the queue.
    uart_driver_install(UART_NUM_2, BUF_SIZE * 2, BUF_SIZE * 2, 20,
                        &uart0_queue, 0);
    uart_param_config(UART_NUM_2, &uart_config);

    // Set UART log level
    esp_log_level_set(TAG, ESP_LOG_INFO);
    // Set UART pins (using UART0 default pins ie no changes.)
    uart_set_pin(UART_NUM_2, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE,
                 UART_PIN_NO_CHANGE);

    gpio_pad_select_gpio(PIN_BUTTON);
    gpio_set_direction(PIN_BUTTON, GPIO_MODE_INPUT);

    gpio_pulldown_en(PIN_BUTTON);
    gpio_pullup_dis(PIN_BUTTON);

    // Set uart pattern detect function.
    //  uart_enable_pattern_det_baud_intr(UART_NUM_2, '+', PATTERN_CHR_NUM, 9,
    //  0, 0); //
    // Reset the pattern queue length to record at most 20 pattern positions.
    uart_pattern_queue_reset(UART_NUM_2, 20);

    // Create a task to handler UART event from ISR
    xTaskCreate(uart_event_task, "uart_event_task", 2048, NULL, 12, NULL);

    xTaskCreate(leitura_button_task, "leitura_button_task", 2048, NULL, 10, NULL);

}
