//
// Created by urdmi on 25.11.2023.
//

#include "wifi_esp.h"

#define EXAMPLE_ESP_WIFI_SSID      "MTS_GPON_3BF6"
#define EXAMPLE_ESP_WIFI_PASS      "CdARCCTH"
#define EXAMPLE_ESP_MAXIMUM_RETRY  5

static EventGroupHandle_t s_wifi_event_group;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG_W = "wifi station";

static int s_retry_num = 0;

static QueueHandle_t* _queue_message_to_send;
static esp_mqtt_client_handle_t* _client_publish;
static esp_mqtt_client_handle_t* _client_subscribe;

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG_W, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG_W,"connect to the AP fail");
#if defined ESP_PUBLISHER
        leds_flash(LED_RED, 500);
#endif
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG_W, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
#if defined ESP_PUBLISHER
        leds_flash(LED_BLUE, 500);
#elif defined ESP_MQTT_ADAPTER
        leds_flash(LED_GREEN, 500);
        mqtt_app_start(_client_publish, _client_subscribe, _queue_message_to_send);
//        send_status_mqtt_adapter(STATUS_WIFI_IS_CONNECT);

#endif
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}
#if defined ESP_PUBLISHER
void wifi_init_sta(esp_mqtt_client_handle_t* client_publish, esp_mqtt_client_handle_t* client_subscribe)
#elif defined ESP_MQTT_ADAPTER
void wifi_init_sta(QueueHandle_t* queue_message_to_send, esp_mqtt_client_handle_t* client_publish, esp_mqtt_client_handle_t* client_subscribe)
#endif

{
    _client_publish = client_publish;
    _client_subscribe = client_subscribe;
#if defined ESP_PUBLISHER
#elif defined ESP_MQTT_ADAPTER
    _queue_message_to_send = queue_message_to_send;
#endif

    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
//    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
            .sta = {
                    .ssid = EXAMPLE_ESP_WIFI_SSID,
                    .password = EXAMPLE_ESP_WIFI_PASS,
                    .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    send_status_mqtt_adapter(STATUS_CONNECTING_WIFI);

    ESP_LOGI(TAG_W, "wifi_init_sta finished.");

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG_W, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG_W, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else {
        ESP_LOGE(TAG_W, "UNEXPECTED EVENT");
    }
}
