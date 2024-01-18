
#include "leds.h"
#include "string.h"
#include "nvs_flash.h"
#include "wifi_esp.h"
#include "mqtt_esp.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "dht11.h"
#include "di.h"
#include "photosensor.h"
#include "freertos/queue.h"
#include "secret.h"

#define PIN_SENSOR_TEMPER_DATA 9
#define PIN_SENSOR_TEMPER_POWER 2
#define PIN_DI 7

//ADC1_1

static SemaphoreHandle_t semaphore_wake_up_dht, semaphore_wake_up_di, semaphore_wake_up_photo;
static int payload_state;

void wake_up_actions(void );
void app_mqtt_connected(esp_mqtt_client_handle_t *active_client, esp_mqtt_client_handle_t *publish_client, esp_mqtt_client_handle_t *subscribe_client);
void app_mqtt_rcv_data(esp_mqtt_event_handle_t event);

void app_main(void)
{
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    semaphore_wake_up_dht = xSemaphoreCreateBinary();
    semaphore_wake_up_di = xSemaphoreCreateBinary();
    semaphore_wake_up_photo = xSemaphoreCreateBinary();

    leds_init();
    dht11_init(PIN_SENSOR_TEMPER_POWER, PIN_SENSOR_TEMPER_DATA, mqtt_esp_publish_message, &semaphore_wake_up_dht);
    di_init(PIN_DI, mqtt_esp_publish_message, &semaphore_wake_up_di, &payload_state);
    light_sensor_init(ADC_CHANNEL_1, mqtt_esp_publish_message, &semaphore_wake_up_photo);

    mqtt_esp_init((char*)secret_root_ca,
                  (char*)secret_cert_devices,(char*)secret_key_devices,
                  (char*)secret_cert_registries, (char*)secret_key_registries,
                  app_mqtt_connected,
                  app_mqtt_rcv_data);

    wifi_init_sta((char*)secret_name_wf, (char*)secret_pass_wf,mqtt_esp_mqtt_app_start);
    ESP_ERROR_CHECK(esp_netif_init());

    while (1){ ; }
}

void wake_up_actions(void ){
    xSemaphoreGive(semaphore_wake_up_dht);
    xSemaphoreGive(semaphore_wake_up_photo);
    xSemaphoreGive(semaphore_wake_up_di);
}

void app_mqtt_connected(esp_mqtt_client_handle_t *active_client, esp_mqtt_client_handle_t *publish_client, esp_mqtt_client_handle_t *subscribe_client) {

    if (active_client == publish_client) {

        ESP_LOGI("TAG_MQTT", "MQTT_PUBLISHER_CONNECTED");
        xTaskCreate(dht11_vTask_read, "DHT11", 4096, NULL, 20, NULL);
        xTaskCreate(di_vTask, "di_vTask", 2048, NULL, 10, NULL);
        xTaskCreate(light_sensor_vTask, "light_sensor_vTask", 2048, NULL, 10, NULL);
        esp_mqtt_client_start(*subscribe_client);

    } else if (active_client == subscribe_client) {

        ESP_LOGI("TAG_MQTT", "MQTT_SUBSCRIBE_CONNECTED");
        esp_mqtt_client_subscribe(*subscribe_client, "gb_iot/2950_UDA/onpayload", 0);
        esp_mqtt_client_subscribe(*subscribe_client, "gb_iot/2950_UDA/wake_up", 0);

    }
}

void app_mqtt_rcv_data(esp_mqtt_event_handle_t event){

    ESP_LOGI("TAG_MQTT", "MQTT_EVENT_DATA");
    printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
    printf("DATA=%.*s\r\n", event->data_len, event->data);
    leds_flash(LED_GREEN, 100);

    char incom_topic[50] = {'\0',};
    sprintf(incom_topic, "%.*s", event->topic_len, event->topic);

    char my_topic[] = "gb_iot/2950_UDA/onpayload";
    if (strcmp(incom_topic, my_topic) == 0) {
        if (event->data[0] == '0') {
            payload_state = 0;
            gpio_set_level(LED_WHITE, 0);

        } else {
            payload_state = 1;
            gpio_set_level(LED_WHITE, 1);
        }
    }

    sprintf(my_topic, "gb_iot/2950_UDA/wake_up");
    if (strcmp(incom_topic, my_topic) == 0) {
        ESP_LOGW("!!!!!!!!!!!", "Wake up event! Publish all data!");
        wake_up_actions();
    }


}