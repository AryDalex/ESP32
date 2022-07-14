/*  WiFi softAP Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_spiffs.h"
#include "esp_vfs.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_http_server.h"

#include <stdio.h>
#include "protocol_examples_common.h"
#include "mdns.h"
#include "cJSON.h"
#include "freertos/event_groups.h"
#include "linked_list.h"
/* The examples use WiFi configuration that you can set via project configuration menu.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/

// http://192.168.4.1/
#define EXAMPLE_ESP_WIFI_SSID      "ESP32"
#define EXAMPLE_ESP_WIFI_PASS      "1234567890"
#define EXAMPLE_ESP_WIFI_CHANNEL   1
#define EXAMPLE_MAX_STA_CONN       4

static const char *TAG = "wifi softAP";

/******************** Server *******************/
static const char *TAGSERVER = "serverRandom.c";
static httpd_handle_t server = NULL;

//Altere o nome do dispositivo
#define NOME    "arydalex"
#define MDNS_NAME  NOME "-esp32"
#define INSTANCE_NAME NOME "_esp32_thing"

static esp_err_t on_default_url(httpd_req_t *req)
{
    ESP_LOGI(TAGSERVER, "URL: %s", req->uri);
    httpd_resp_sendstr(req, "hello world");
    return ESP_OK;
}

#define WS_MAX_SIZE 4096
/*******************************************/

esp_netif_t *my_ap = NULL;
QueueHandle_t xQueueHttp;

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    my_ap = esp_netif_create_default_wifi_ap();
    esp_netif_dhcpc_stop(my_ap);

    // esp_netif_ip_info_t ip_info;

    // IP4_ADDR(&ip_info.ip, 192, 168, 4, 1);
   	// IP4_ADDR(&ip_info.gw, 192, 168, 4, 1);
   	// IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);

    // esp_netif_set_ip_info(my_ap, &ip_info);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d", EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
}

/******************** Web Socket *******************/
esp_err_t send_ws_message(char *message)
{
    esp_err_t ret = ESP_OK;
    struct node* first = getFirst();
    
    for( int i = 0; i < length(); i++) 
    {
        httpd_ws_frame_t ws_message = {.final = true,
                                    .fragmented = false,
                                    .len = strlen(message),
                                    .payload = (uint8_t *)message,
                                    .type = HTTPD_WS_TYPE_TEXT};

       ret = httpd_ws_send_frame_async(server, first->socket, &ws_message);
       first = first->next;
    }

    return ret;
}

static esp_err_t on_web_socket_url(httpd_req_t *req)
{    
    if (req->method == HTTP_GET)
        return ESP_OK;

    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    ws_pkt.payload = malloc(WS_MAX_SIZE);
    httpd_ws_recv_frame(req, &ws_pkt, WS_MAX_SIZE);
    printf("ws payload: %.*s\n", ws_pkt.len, ws_pkt.payload);
    
    free(ws_pkt.payload);
    return ESP_OK;
}

/*******************************************/

esp_err_t open_socket_handler(httpd_handle_t server, int sockfd) {
    ESP_LOGI(TAGSERVER, "client %d connected", sockfd);
    insertFirst(sockfd);
    return ESP_OK;
}

void closed_socket_handler(httpd_handle_t server, int sockfd) {
    struct node* socket = deleteNode(sockfd);
    if(socket)
        free(socket);
    ESP_LOGI(TAGSERVER, "client %d disconnected", sockfd);
}

static void init_server()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.close_fn = closed_socket_handler; // cria quando fecha coneccao
    config.open_fn = open_socket_handler; // cria quando abre coneccao

    ESP_ERROR_CHECK(httpd_start(&server, &config));

    httpd_uri_t default_url = {
        .uri = "/", .method = HTTP_GET, .handler = on_default_url};
    httpd_register_uri_handler(server, &default_url);

    httpd_uri_t web_socket_url = {.uri = "/ws",
                                  .method = HTTP_GET,
                                  .handler = on_web_socket_url,
                                  .is_websocket = true};
    httpd_register_uri_handler(server, &web_socket_url);
}

void start_mdns_service()
{
    mdns_init();
    mdns_hostname_set(MDNS_NAME);
    mdns_instance_name_set(INSTANCE_NAME);
}

void app_main(void)
{
    ESP_LOGI(TAGSERVER, "[APP] Startup..");
    ESP_LOGI(TAGSERVER, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAGSERVER, "[APP] IDF version: %s", esp_get_idf_version());

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // inicia servidor
    init_server();

    esp_netif_ip_info_t ip;
    
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();

    esp_netif_get_ip_info(my_ap, &ip);
    ESP_LOGI(TAG, "ip = " IPSTR, IP2STR(&ip.ip));

    while(1) {
        // cria object json
        cJSON *object = cJSON_CreateObject();
        // cria dado string
        cJSON_AddStringToObject(object, "topic", "random_number");
        // cria numeros aleatorios | 1 a 100
        cJSON_AddNumberToObject(object, "payload", esp_random()%99 + 1);
        // resebe valor para enviar
        char *message = cJSON_Print(object);
        // printa valor
        printf("message: %s\n", message);
        // 
        send_ws_message(message);
        // deleta dado da memoria
        cJSON_Delete(object);
        // 
        free(message);

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}