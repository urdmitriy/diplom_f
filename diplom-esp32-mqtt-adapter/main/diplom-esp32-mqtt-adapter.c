#include <stdio.h>
#include "leds.h"
#include "string.h"
#include "nvs_flash.h"
#include "wifi_esp.h"
#include "mqtt_esp.h"
#include "esp_mac.h"
#include "esp_wifi.h"

#if defined ESP_MQTT_ADAPTER
#include "uart_esp.h"
#endif

esp_mqtt_client_handle_t mqtt_client;

void app_main(void)
{
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    leds_init();
    wifi_init_sta();
    ESP_ERROR_CHECK(esp_netif_init());
    mqtt_app_start(&mqtt_client);

#if defined ESP_MQTT_ADAPTER
    uart_init();
#endif

    while (1){ ; }
}
