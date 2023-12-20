#include <stdio.h>
#include "leds.h"
#include "string.h"
#include "nvs_flash.h"
#include "wifi_esp.h"
#include "mqtt_esp.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "uart_esp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

void app_main(void)
{
    static esp_mqtt_client_handle_t mqtt_client;
    static QueueHandle_t queue_message_to_send = NULL;
    queue_message_to_send = xQueueCreate(50, sizeof(uart_data_t));

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    uart_init(uart_data_handler, &queue_message_to_send);
    leds_init();
    wifi_init_sta(&queue_message_to_send, &mqtt_client);
    ESP_ERROR_CHECK(esp_netif_init());


    while (1){ ; }
}
