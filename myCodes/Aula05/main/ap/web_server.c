#include <stdio.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "web_server.h"
#include "html_support.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_http_client.h"

const char* TAG = "web_server.c";

// static EventGroupHandle_t wifi_events;
// #define WIFI_CONNECTED_BIT BIT0
// #define WIFI_FAIL_BIT BIT1
// #define MAX_RETRY 10
// static int retry_cnt = 0;
// static const char *TAG_WIFI = "wifi_app";
// static void request_page(void *);
// static esp_err_t handle_http_event(esp_http_client_event_t *);
// static void handle_wifi_connection(void *, esp_event_base_t,
// int32_t, void *);

static const char *HTML_FORM =
"<html><body><h1>WEB Form Demo using ESP-IDF</h1>"
"<form action=\"/\" method=\"post\">"
"<label for=\"ssid\">Local SSID:</label><br>"
"<input type=\"text\" id=\"ssid\" name=\"ssid\"><br>"
"<label for=\"pwd\">Password:</label><br>"
"<input type=\"text\" id=\"pwd\" name=\"pwd\"><br>"
"<input type=\"submit\" value=\"Submit\">"
"</form></body></html>";

static esp_err_t root_get_handler(httpd_req_t *req);
static esp_err_t root_post_handler(httpd_req_t *req);
static esp_err_t handle_http_get(httpd_req_t *req);
static esp_err_t handle_http_post(httpd_req_t *req);

/* HTTP get handler */
static esp_err_t root_get_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "root_get_handler req->uri=[%s]", req->uri);
	esp_err_t err;

	// Send HTML header
	httpd_resp_sendstr_chunk(req, "<!DOCTYPE html><html>");
	Text2Html(req, "/html/head.html");

	httpd_resp_sendstr_chunk(req, "<body>");
	httpd_resp_sendstr_chunk(req, "<h1>ESP-IDF wi-fi</h1>");

	httpd_resp_sendstr_chunk(req, "<form action=\"/\" method=\"post\">");

	httpd_resp_sendstr_chunk(req, "<label for=\"ssid\">Name:</label><br>");
	httpd_resp_sendstr_chunk(req, "<input type=\"text\" id=\"name\" name=\"name\"><br>");

	httpd_resp_sendstr_chunk(req, "<label for=\"pwd\">Password:</label><br>");
	httpd_resp_sendstr_chunk(req, "<input type=\"text\" id=\"pwd\" name=\"pwd\"><br>");

	httpd_resp_sendstr_chunk(req, "<input type=\"submit\" value=\"Submit\">");

	httpd_resp_sendstr_chunk(req, "</form>");

	/* Send remaining chunk of HTML file to complete it */
	httpd_resp_sendstr_chunk(req, "</body></html>");

	/* Send empty chunk to signal HTTP response completion */
	httpd_resp_sendstr_chunk(req, NULL);

	return ESP_OK;
}

/* HTTP post handler */
static esp_err_t root_post_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "root_post_handler req->uri=[%s]", req->uri);
	ESP_LOGI(TAG, "root_post_handler content length %d", req->content_len);
	
    if(req->content_len > 0) 
    {
        char*  buf = malloc(req->content_len + 1);
        size_t off = 0;
        while (off < req->content_len) {
            /* Read data received in the request */
            int ret = httpd_req_recv(req, buf + off, req->content_len - off);
            if (ret <= 0) {
                if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                    httpd_resp_send_408(req);
                }
                free (buf);
                return ESP_FAIL;
            }
            off += ret;
            ESP_LOGI(TAG, "root_post_handler recv length %d", ret);
        }
        buf[off] = '\0';
        ESP_LOGI(TAG, "root_post_handler buf=[%s]", buf);

        ESP_LOGI(TAG, "%.*s", off, buf);
        post_messages_t post_message;
        post_message.lenght = off;
        strncpy(post_message.message, buf, (off < 128)? (off): 128);

        if (xQueueSend(xQueueHttp, &post_message, portMAX_DELAY) != pdPASS) {
            ESP_LOGE(TAG, "xQueueSend Fail");
        }
        free(buf);
    }
    
	/* Redirect onto root to see the updated file list */
	httpd_resp_set_status(req, "303 See Other");
	httpd_resp_set_hdr(req, "Location", "/");
#ifdef CONFIG_EXAMPLE_HTTPD_CONN_CLOSE_HEADER
	httpd_resp_set_hdr(req, "Connection", "close");
#endif
	httpd_resp_sendstr(req, "post successfully");
	return ESP_OK;
}
/* favicon get handler */
static esp_err_t favicon_get_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "favicon_get_handler req->uri=[%s]", req->uri);
	return ESP_OK;
}

void start_webserver(void)
{    
    httpd_uri_t _root_get_handler = {
    .uri		 = "/",
    .method		 = HTTP_GET,
    .handler	 = root_get_handler,
    // .user_ctx  = server_data	// Pass server data as context
	};

    /* URI handler for post */
	httpd_uri_t _root_post_handler = {
		.uri		 = "/post",
		.method		 = HTTP_POST,
		.handler	 = root_post_handler,
		// .user_ctx  = server_data	// Pass server data as context
	};

    	/* URI handler for favicon.ico */
	httpd_uri_t _favicon_get_handler = {
		.uri		 = "/favicon.ico",
		.method		 = HTTP_GET,
		.handler	 = favicon_get_handler,
		// .user_ctx  = server_data	// Pass server data as context
	};

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server, &_root_get_handler);
        httpd_register_uri_handler(server, &_root_post_handler);
        httpd_register_uri_handler(server, &_favicon_get_handler);
    }
}

static esp_err_t handle_http_get(httpd_req_t *req)
{
    return httpd_resp_send(req, HTML_FORM, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t handle_http_post(httpd_req_t *req)
{
    char content[100];
    if (httpd_req_recv(req, content, req->content_len) <= 0)
    {
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "%.*s", req->content_len, content);
    return httpd_resp_send(req, "received", HTTPD_RESP_USE_STRLEN);
}

/********************** sta **********************/
// static void init_wifi(void)
// {
//     if (nvs_flash_init() != ESP_OK)
//     {
//         nvs_flash_erase();
//         nvs_flash_init();
//     }

//     wifi_events = xEventGroupCreate();
//     esp_event_loop_create_default();
//     esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &handle_wifi_connection, NULL);
//     esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &handle_wifi_connection, NULL);
//     wifi_config_t wifi_config = {
//         .sta = {
//             .ssid = WIFI_NAME,
//             .password = WIFI_PASS,
//             .threshold.authmode = WIFI_AUTH_WPA2_PSK,
//             .pmf_cfg = {
//                 .capable = true,
//                 .required = false},
//         },
//     };
//     esp_netif_init();
//     esp_netif_create_default_wifi_sta();
//     wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
//     esp_wifi_init(&cfg);
//     esp_wifi_set_mode(WIFI_MODE_STA);
//     esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
//     esp_wifi_start();
//     EventBits_t bits = xEventGroupWaitBits(wifi_events, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
//     if (bits & WIFI_CONNECTED_BIT)
//     {
//         xTaskCreate(request_page, "http_req", 5 *
//         configMINIMAL_STACK_SIZE, NULL, 5, NULL);
//     }
//     else
//     {
//         ESP_LOGE(TAG_WIFI, "failed");
//     }
// }

// static void handle_wifi_connection(void *arg, esp_event_base_t event_base, 
//                                 int32_t event_id, void *event_data)
// {
//     if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
//     {
//         esp_wifi_connect();
//     }
//     else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
//     {
//         if (retry_cnt++ < MAX_RETRY)
//         {
//             esp_wifi_connect();
//             ESP_LOGI(TAG_WIFI, "wifi connect retry: %d", retry_cnt);
//         }
//         else
//         {
//             xEventGroupSetBits(wifi_events, WIFI_FAIL_BIT);
//         }
//     }
//     else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
//     {
//         ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
//         ESP_LOGI(TAG_WIFI, "ip: %d.%d.%d.%d", IP2STR(&event->ip_info.ip));
//         retry_cnt = 0;
//         xEventGroupSetBits(wifi_events, WIFI_CONNECTED_BIT);
//     }
// }

// static void request_page(void *arg)
// {
//     esp_http_client_config_t config = 
//     {
//         .url = "https://www.google.com/",
//         .event_handler = handle_http_event,
//     };
//     esp_http_client_handle_t client = esp_http_client_init(&config);
//     if (esp_http_client_perform(client) != ESP_OK)
//     {
//         ESP_LOGE(TAG_WIFI, "http request failed");
//     }
//     esp_http_client_cleanup(client);
//     vTaskDelete(NULL);
// }

// static esp_err_t handle_http_event(esp_http_client_event_t *http_event)
// {
//     switch (http_event->event_id)
//     {
//         case HTTP_EVENT_ON_DATA:
//             printf("%.*s\n", http_event->data_len, 
//                     (char *)http_event->data);
//         break;

//         default:
//         break;
//     }
//     return ESP_OK;
// }

// void app_main(void)
// {
//     init_wifi();
// }