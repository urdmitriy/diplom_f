//
// Created by urdmi on 16.12.2023.
//

#include "target.h"
#include "photosensor.h"
#include "esp_log.h"


static adc_channel_t _adc_channel;
static adc_oneshot_unit_handle_t adc1_handle;
static mqtt_publish_app _mqttPublishApp;

void light_sensor_init(adc_channel_t channel, mqtt_publish_app mqttPublishApp){
    _adc_channel = channel;
    _mqttPublishApp = mqttPublishApp;
    //-------------ADC1 Init---------------//
    adc_oneshot_unit_init_cfg_t init_config1 = {
            .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&init_config1, &adc1_handle);

    //-------------ADC1 Config---------------//
    adc_oneshot_chan_cfg_t config = {
            .bitwidth = ADC_BITWIDTH_DEFAULT,
            .atten = ADC_ATTEN_DB_11,
    };
    adc_oneshot_config_channel(adc1_handle, _adc_channel, &config);
}

void light_sensor_vTask(void *arg) {

    static int adc_value, adc_value_old;
    static uint64_t time_last_send_data_light;

    for (;;) {

        adc_oneshot_read(adc1_handle, _adc_channel, &adc_value);
        adc_value = adc_value / 100 * 100;

        if ((adc_value != adc_value_old) || ((esp_timer_get_time() - time_last_send_data_light) > MAX_DELAY_SEND_DATA)) {
            ESP_LOGI("ADC", "New val = %d, old val = %d", adc_value, adc_value_old);
            char message[10], topic[100];
            sprintf(message, "%d", adc_value);
            sprintf(topic, BASE_TOPIC_NAME, "insol");
            _mqttPublishApp(topic, message);
            ESP_LOGI("ADC", "ADC_READ = %d", adc_value);
            adc_value_old = adc_value;
            time_last_send_data_light = esp_timer_get_time();
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}