#include <stdio.h>
#include "leds.h"
#include "string.h"
#include "nvs_flash.h"
#include "wifi_esp.h"
#include "mqtt_esp.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "uart_esp.h"
#include "secret.h"

void app_main(void)
{
    vTaskDelay(pdMS_TO_TICKS(3000));
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    uart_init(mqtt_esp_publish_message, mqtt_esp_mqtt_subscribe,mqtt_esp_mqtt_unsubscribe);
    leds_init();
    mqtt_esp_init((char*)secret_root_ca,
                  (char*)secret_cert_devices,(char*)secret_key_devices,
                  (char*)secret_cert_registries, (char*)secret_key_registries,
                  uart_esp_uart_data_handler);

    wifi_init_sta((char*)secret_name_wf, (char*)secret_pass_wf, mqtt_esp_mqtt_app_start);
    ESP_ERROR_CHECK(esp_netif_init());


    while (1){ ; }
}
