/* MQTT (over TCP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "freertos/event_groups.h"

static const char *TAG = "MQTT_EXAMPLE";

typedef struct receive {
    char topic[100];
    char data[100];
} rcv_t;

// WiFi network info.
char ssid[] = "dilson";
char wifiPassword[] = "Cl@udionor";

// Cayenne authentication info. This should be obtained from the Cayenne Dashboard.
#define BROKER "mqtt://mqtt.mydevices.com"
#define USERNAME "ba5941b0-e29c-11ec-8da3-474359af83d7"
#define PASSWORD "5e991267717b88db816b5d37beb01006668c1eea"
#define CLIENT_ID "3b3589b0-e29d-11ec-8c44-371df593ba58"

// https://developers.mydevices.com/cayenne/docs/cayenne-mqtt-api/#cayenne-mqtt-api-supported-data-types
// https://developers.mydevices.com/cayenne/docs/cayenne-mqtt-api/#cayenne-mqtt-api-mqtt-messaging-topics-examples

#define MAIN_TOPIC  "v1/" USERNAME "/things/" CLIENT_ID
#define RSP_TOPIC MAIN_TOPIC "/response"

#define RSSI_CH "1"
#define MEM_CH "2"

#define RSSI_TOPIC MAIN_TOPIC "/data/" RSSI_CH
#define RSSI_MSG_PREFIX "rssi,dbm="

#define MEM_TOPIC MAIN_TOPIC "/data/" MEM_CH
#define MEM_MSG_PREFIX "storage,byte="

#define WIFI_CONNECTED_BIT BIT0
#define MQTT_CONNECTED_BIT BIT1

EventGroupHandle_t events;
esp_mqtt_client_handle_t client;
TaskHandle_t send_handle;

static void log_error_if_nonzero(const char * message, int errorCode)
{
    if (errorCode != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, errorCode);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) 
{
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t) event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    rcv_t rcv;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED: // conectou 
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            xEventGroupSetBits(events, MQTT_CONNECTED_BIT); // monitora se esta conectado
            break;
        case MQTT_EVENT_DISCONNECTED: // nao conectou
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            xEventGroupClearBits(events, MQTT_CONNECTED_BIT); // monitora se nao esta conectado
            break;
        case MQTT_EVENT_SUBSCRIBED: // inscrevel
            ESP_LOGI(TAG, "msg_id=%d | MQTT_EVENT_SUBSCRIBED", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED: // desinscrevel 
            ESP_LOGI(TAG, "msg_id=%d | MQTT_EVENT_UNSUBSCRIBED", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED: // enviou um dato
            ESP_LOGI(TAG, "msg_id=%d | MQTT_EVENT_PUBLISHED", event->msg_id);
            break;
        case MQTT_EVENT_DATA: 
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR: // caso ocorra um erro
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
                log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
                log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
                ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
            }
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
}

static void mqtt_app_start(void)
{
    // configuracoes da mqtt
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = BROKER,
        .port = 1883,
        .username = USERNAME,
        .password = PASSWORD,
        .client_id = CLIENT_ID
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client); // cria estrutura de enventos, mostra eventos referente ao client
    esp_mqtt_client_start(client); // inicia 
}

void send_task(void *param)
{
    wifi_ap_record_t ap_info;
    char message[20] = {0};
    while(1)
    {        
        EventBits_t bits = xEventGroupWaitBits(events, MQTT_CONNECTED_BIT,
                    pdFALSE, pdFALSE, 0);
        if(bits & MQTT_CONNECTED_BIT) 
        { // se esta conectado
            if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK)
            { // envia mensagem
                sprintf(message, RSSI_MSG_PREFIX"%d", ap_info.rssi);
                int msg_id = esp_mqtt_client_publish(client, RSSI_TOPIC, message, strlen(message), 1, 0); // define qual e aonde enviar a mensagem
                ESP_LOGW(TAG, "msg_id=%d | sending %.*s", msg_id, strlen(message),message); // envia mensagem
            }

            memset(message, 0, sizeof(message));
            sprintf(message, MEM_MSG_PREFIX"%d", esp_get_free_heap_size());
            int msg_id = esp_mqtt_client_publish(client, MEM_TOPIC, message, strlen(message), 1, 0);
            ESP_LOGW(TAG, "msg_id=%d | sending %.*s", msg_id, strlen(message),message);
        }
        
        vTaskDelay(pdMS_TO_TICKS(15000)); // envia mensagens de 15s em 15s
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup.."); // printa 
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size()); //esp_get_free_heap_size() = mostra quanto ta na memoria
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init()); // 
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect()); // baixa preta, conecta e mostra que foi conectado
    events = xEventGroupCreate(); //

    xTaskCreate(send_task, "send_task", 2048, NULL, 12, &send_handle);

    mqtt_app_start();
}