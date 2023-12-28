
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

#define PIN_SENSOR_TEMPER 9
#define PIN_DI 7
//ADC1_1

esp_mqtt_client_handle_t mqtt_client_publish;
esp_mqtt_client_handle_t mqtt_client_subscibe;

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
    dht11_init(PIN_SENSOR_TEMPER, &mqtt_client_publish);
    di_init(PIN_DI, &mqtt_client_publish);
    light_sensor_init(ADC_CHANNEL_1, &mqtt_client_publish);
    wifi_init_sta(&mqtt_client_publish, &mqtt_client_subscibe);
    ESP_ERROR_CHECK(esp_netif_init());
    mqtt_app_start(&mqtt_client_publish, &mqtt_client_subscibe);

    while (1){ ; }
}
