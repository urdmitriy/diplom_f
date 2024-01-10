//
// Created by urdmi on 25.11.2023.
//

#include "wifi_esp.h"

#define EXAMPLE_ESP_MAXIMUM_RETRY  5

static EventGroupHandle_t s_wifi_event_group;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG_W = "wifi station";

static int s_retry_num = 0;
static mqtt_start_app _mqtt_start;

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
#ifdef ESP_MQTT_ADAPTER
        send_status_mqtt_adapter("Connect to wifi complete");
#endif
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
        _mqtt_start();
#elif defined ESP_MQTT_ADAPTER
        leds_flash(LED_GREEN, 500);
        _mqtt_start();
#endif
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(char* essid, char* wifipass, mqtt_start_app mqttStartApp) {

    _mqtt_start = mqttStartApp;

    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
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
                    .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            },

    };

    memcpy(wifi_config.sta.ssid, (uint8_t*)essid, strlen(essid));
    memcpy(wifi_config.sta.password, (uint8_t*)wifipass, strlen (wifipass));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

#ifdef ESP_MQTT_ADAPTER
    send_status_mqtt_adapter("Connecting to wifi");
#endif
    ESP_LOGI(TAG_W, "wifi_init_sta finished.");

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG_W, "connected to ap SSID:%s password:%s",
                 essid, wifipass);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG_W, "Failed to connect to SSID:%s, password:%s",
                 essid, wifipass);
    } else {
        ESP_LOGE(TAG_W, "UNEXPECTED EVENT");
    }
}
