/* UART Events Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "arduino.h" //

static const char *TAG = "uart_events";

#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_16)

#define PATTERN_CHR_NUM    (3)         

#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)

#define stx 2 // ponto inicio
#define etx 3 // ponto final

xQueueHandle verificar;
//xQueueHandle transmitir;

int counter;
uint8_t data[14];
uint8_t hexBlock1, hexBlock2, hexBlock3, hexBlock4, hexBlock5;
uint8_t hexCalculatedChecksum, hexChecksum;

int loop(uint8_t *value, size_t event_size){
    //if (event_size > 0) {
            //data[counter] = value.read();

        if(value > 13) {
            //we read the whole message, so reset counter
            counter = 0;
            //check if start of text and end of text is correct
            if(data[0] == stx && data[13] == etx) {
                printf("Caracteres STX e ETX corretamente recebidos");
                printf("ID: ");
                //show ID
                for(int x = 1; x < 11; x++) {
                    printf("%d", data[x]);
                }

                printf("\n");
                printf("Checksum: ");
                //show checksum
                printf("%d", data[11]);
                printf("%d", data[12]);

                //Hex ID blocks. Two transmitted Bytes form one Hex ID block.
                //Hex ID blocks:      6   2  |  E   3  |  0   8  |  6   C  |  E   D
                //Transmitted Bytes: 36H 32H | 45H 33H | 30H 38H | 36H 43H | 45H 44H
                hexBlock1 = AsciiCharToNum(data[1])*16 + AsciiCharToNum(data[2]);
                hexBlock2 = AsciiCharToNum(data[3])*16 + AsciiCharToNum(data[4]);
                hexBlock3 = AsciiCharToNum(data[5])*16 + AsciiCharToNum(data[6]);
                hexBlock4 = AsciiCharToNum(data[7])*16 + AsciiCharToNum(data[8]);
                hexBlock5 = AsciiCharToNum(data[9])*16 + AsciiCharToNum(data[10]);
                //Transmitted checksum.
                hexChecksum = AsciiCharToNum(data[11])*16 + AsciiCharToNum(data[12]);
                //XOR algorithm to calculate checksum of ID blocks.
                hexCalculatedChecksum = hexBlock1 ^ hexBlock2 ^ hexBlock3 ^ hexBlock4 ^ hexBlock5;
             
                if (hexCalculatedChecksum == hexChecksum){
                    printf("Checksum calculado confere com checksum transmitido.");
                }
                else {
                    printf("Checksum calculado NAO confere com checksum transmitido. Dados corrompidos!");
                }
            }
        }
    }
}

static QueueHandle_t uart0_queue;

static void uart_event_task(void *pvParameters) {
    uart_event_t event;
    size_t buffered_size;
    uint8_t* dtmp = (uint8_t*) malloc(RD_BUF_SIZE);
    static const char *RX_TASK_TAG = "RX_TASK";
    for(;;) {
        if(xQueueReceive(uart0_queue, (void * )&event, (portTickType)portMAX_DELAY)) {
            bzero(dtmp, RD_BUF_SIZE);
            //ESP_LOGI(TAG, "uart[%d] event:",UART_NUM_2);
            switch(event.type) {
                
                case UART_DATA:
                    ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
                    uart_read_bytes(UART_NUM_2, dtmp, event.size, portMAX_DELAY);

                    if (dtmp > 0) {
                        //data[counter] = value.read();

                        if(value > 13) {
                            //we read the whole message, so reset counter
                            counter = 0;
                            //check if start of text and end of text is correct
                            if(data[0] == stx && data[13] == etx) {
                                printf("Caracteres STX e ETX corretamente recebidos");
                                printf("ID: ");
                                //show ID
                                for(int x = 1; x < 11; x++) {
                                    printf("%d", data[x]);
                                }

                                printf("\n");
                                printf("Checksum: ");
                                //show checksum
                                printf("%d", data[11]);
                                printf("%d", data[12]);

                                //Hex ID blocks. Two transmitted Bytes form one Hex ID block.
                                //Hex ID blocks:      6   2  |  E   3  |  0   8  |  6   C  |  E   D
                                //Transmitted Bytes: 36H 32H | 45H 33H | 30H 38H | 36H 43H | 45H 44H
                                hexBlock1 = AsciiCharToNum(data[1])*16 + AsciiCharToNum(data[2]);
                                hexBlock2 = AsciiCharToNum(data[3])*16 + AsciiCharToNum(data[4]);
                                hexBlock3 = AsciiCharToNum(data[5])*16 + AsciiCharToNum(data[6]);
                                hexBlock4 = AsciiCharToNum(data[7])*16 + AsciiCharToNum(data[8]);
                                hexBlock5 = AsciiCharToNum(data[9])*16 + AsciiCharToNum(data[10]);
                                //Transmitted checksum.
                                hexChecksum = AsciiCharToNum(data[11])*16 + AsciiCharToNum(data[12]);
                                //XOR algorithm to calculate checksum of ID blocks.
                                hexCalculatedChecksum = hexBlock1 ^ hexBlock2 ^ hexBlock3 ^ hexBlock4 ^ hexBlock5;
                            
                                if (hexCalculatedChecksum == hexChecksum){
                                    printf("Checksum calculado confere com checksum transmitido.");
                                }
                                else {
                                    printf("Checksum calculado NAO confere com checksum transmitido. Dados corrompidos!");
                                }
                            }
                        }
                    }

                    if(loop(dtmp, event.size, data[counter])){
                        //uart_write_bytes(UART_NUM_2, (const char*) dtmp, event.size);
                        ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, dtmp, event.size, ESP_LOG_INFO);
                       // xQueueSend(verifica, dtmp, 100 / portTICK_PERIOD_MS);
                        // printf("essa Ã© a fila %.*s", event.size, dtmp); ///
                    }
                    break;

                case UART_FIFO_OVF:
                    ESP_LOGI(TAG, "hw fifo overflow");
                
                    uart_flush_input(UART_NUM_2);
                    xQueueReset(uart0_queue);
                    break;
                //Event of UART ring buffer full
                case UART_BUFFER_FULL:
                    ESP_LOGI(TAG, "ring buffer full");
                    
                    uart_flush_input(UART_NUM_2);
                    xQueueReset(uart0_queue);
                    break;
                //Event of UART RX break detected
                case UART_BREAK:
                    ESP_LOGI(TAG, "uart rx break");
                    break;
                //Event of UART parity check error
                case UART_PARITY_ERR:
                    ESP_LOGI(TAG, "uart parity error");
                    break;
                //Event of UART frame error
                case UART_FRAME_ERR:
                    ESP_LOGI(TAG, "uart frame error");
                    break;
                //UART_PATTERN_DET
                case UART_PATTERN_DET:
                    uart_get_buffered_data_len(UART_NUM_2, &buffered_size);
                    int pos = uart_pattern_pop_pos(UART_NUM_2);
                    ESP_LOGI(TAG, "[UART PATTERN DETECTED] pos: %d, buffered size: %d", pos, buffered_size);
                    if (pos == -1) {
                       
                        uart_flush_input(UART_NUM_2);
                    } else {
                        uart_read_bytes(UART_NUM_2, dtmp, pos, 100 / portTICK_PERIOD_MS);
                        uint8_t pat[PATTERN_CHR_NUM + 1];
                        memset(pat, 0, sizeof(pat));
                        uart_read_bytes(UART_NUM_2, pat, PATTERN_CHR_NUM, 100 / portTICK_PERIOD_MS);
                        ESP_LOGI(TAG, "read data: %s", dtmp);
                        ESP_LOGI(TAG, "read pat : %s", pat);
                    }
                    break;
                //Others
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
    //Install UART driver, and get the queue.
    uart_driver_install(UART_NUM_2, BUF_SIZE * 2, BUF_SIZE * 2, 20, &uart0_queue, 0);
    uart_param_config(UART_NUM_2, &uart_config);

    //Set UART log level
    esp_log_level_set(TAG, ESP_LOG_INFO);
    //Set UART pins (using UART0 default pins ie no changes.)
    uart_set_pin(UART_NUM_2, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    //Set uart pattern detect function.
    // uart_enable_pattern_det_baud_intr(UART_NUM_2, '+', PATTERN_CHR_NUM, 9, 0, 0); //
    //Reset the pattern queue length to record at most 20 pattern positions.
    uart_pattern_queue_reset(UART_NUM_2, 20);

    //Create a task to handler UART event from ISR
    xTaskCreate(uart_event_task, "uart_event_task", 2048, NULL, 12, NULL);
}

int AsciiCharToNum(uint8_t data) {
    // First substract 48 to convert the char representation
    // of a number to an actual number.
    data -= '0';
    // If it is greater than 9, we have a Hex character A-F.
    // Substract 7 to get the numeral representation.
    if (data > 9){
        data -= 7;
        return data;
    }
}