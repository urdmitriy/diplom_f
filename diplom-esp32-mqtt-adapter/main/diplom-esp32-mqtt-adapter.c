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

void app_mqtt_connected(esp_mqtt_client_handle_t *active_client, esp_mqtt_client_handle_t *publish_client,
                        esp_mqtt_client_handle_t *subscribe_client);
void app_mqtt_rcv_data(esp_mqtt_event_handle_t event);

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
                  app_mqtt_connected, app_mqtt_rcv_data);

    wifi_init_sta((char*)secret_name_wf, (char*)secret_pass_wf, mqtt_esp_mqtt_app_start);
    ESP_ERROR_CHECK(esp_netif_init());


    while (1){ ; }
}

void app_mqtt_connected(esp_mqtt_client_handle_t *active_client, esp_mqtt_client_handle_t *publish_client, esp_mqtt_client_handle_t *subscribe_client) {

    if (active_client == publish_client) {

        ESP_LOGI("TAG_MQTT", "MQTT_PUBLISHER_CONNECTED");
        esp_mqtt_client_start(*subscribe_client);

    } else if (active_client == subscribe_client) {

        ESP_LOGI("TAG_MQTT", "MQTT_SUBSCRIBE_CONNECTED");
        mqtt_esp_mqtt_subscribe("2950_UDA");

    }

}

void app_mqtt_rcv_data(esp_mqtt_event_handle_t event){

    leds_flash(LED_GREEN, 50);

    ESP_LOGI("TAG_MQTT", "MQTT_EVENT_DATA");
    printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
    printf("DATA=%.*s\r\n", event->data_len, event->data);

    char topic[100];
    char data_topic[10];

    sprintf(topic, "%.*s", event->topic_len, event->topic);
    sprintf(data_topic, "%.*s", event->data_len, event->data);

    uart_data_t data;

    data.data_type = DATA_TYPE_DATA;
    data.id_parametr = (int)mqtt_esp_get_param_name(topic);
    memset((uint8_t*)data.value, '\0', sizeof (data.value));
    sprintf(data.value,"%.*s", event->data_len, event->data);
    data.crc = crc8ccitt((uint8_t*)&data, DATA_SIZE);

    ESP_LOGI("RES", "Data type = %lu, parametr = %d, value = %s, crc = %d", data.data_type, data.id_parametr, data.value, data.crc);
    uart_esp_uart_data_handler((char*)&data);
}