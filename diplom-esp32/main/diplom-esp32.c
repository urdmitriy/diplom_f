
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

#define PIN_SENSOR_TEMPER 9
#define PIN_DI 7

//ADC1_1

static SemaphoreHandle_t semaphore_wake_up_dht, semaphore_wake_up_di, semaphore_wake_up_photo;
static int payload_state;

void wake_up_actions(void );

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
    dht11_init(PIN_SENSOR_TEMPER, mqtt_esp_publish_message, &semaphore_wake_up_dht);
    di_init(PIN_DI, mqtt_esp_publish_message, &semaphore_wake_up_di, &payload_state);
    light_sensor_init(ADC_CHANNEL_1, mqtt_esp_publish_message, &semaphore_wake_up_photo);

    mqtt_esp_init((char*)secret_root_ca,
                  (char*)secret_cert_devices,(char*)secret_key_devices,
                  (char*)secret_cert_registries, (char*)secret_key_registries,
                  uart_esp_uart_data_handler,
                  wake_up_actions, &payload_state);

    wifi_init_sta((char*)secret_name_wf, (char*)secret_pass_wf,mqtt_esp_mqtt_app_start);
    ESP_ERROR_CHECK(esp_netif_init());

    while (1){ ; }
}

void wake_up_actions(void ){
    xSemaphoreGive(semaphore_wake_up_dht);
    xSemaphoreGive(semaphore_wake_up_photo);
    xSemaphoreGive(semaphore_wake_up_di);
}