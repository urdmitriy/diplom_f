//
// Created by urdmi on 25.11.2023.
//

#include "dht11.h"

#define MAX_TIME_SENSOR_WAIT_US 10000

static int _pin_sensor;
static mqtt_publish_app _mqttPublishApp;

void dht11_init(int pin_sensor, mqtt_publish_app mqttPublishApp) {
    _pin_sensor = pin_sensor;
    _mqttPublishApp = mqttPublishApp;

    gpio_reset_pin(_pin_sensor);
    timer_config_t config_timer = {
            .counter_dir = TIMER_COUNT_UP,
            .divider = 80,
            .auto_reload = TIMER_AUTORELOAD_EN,
            .alarm_en = TIMER_ALARM_DIS,
            .counter_en = TIMER_START};
    timer_init(TIMER_GROUP_0, TIMER_0, &config_timer);
}

void dht11_vTask_read(void * pvParameters )
{
    data_sensor_t data_sensor = {.humidity = 0, .temperature = 0, .crc = 0};
    data_sensor_t data_sensor_old = {.humidity = 255, .temperature = 255, .crc = 0};
    static uint64_t time_last_send_data_temperature, time_last_send_data_humidity;

    for( ;; )
    {
        dht11_read(&data_sensor);
        uint8_t crc_calc =
                (uint8_t)(data_sensor.temperature>>8)+
                (uint8_t)(data_sensor.temperature & 0x00FF) +
                (uint8_t)(data_sensor.humidity>>8);
        ESP_LOGI("DHT11", "Temperature: %d.%d, humidity: %d.%d, crc: %d, crc calc: %d.",
                 data_sensor.temperature>>8, data_sensor.temperature & 0x00FF, data_sensor.humidity>>8,
                 data_sensor.humidity & 0x00FF, data_sensor.crc, crc_calc);

        char message[500], topic[300];


        if (data_sensor.temperature == 0 && data_sensor.humidity == 0 && data_sensor.crc == 0){

            sprintf(message, "ERROR");
            sprintf(topic, BASE_TOPIC_NAME, "temp");
            _mqttPublishApp(topic, message);
            sprintf(topic, BASE_TOPIC_NAME, "humidity");
            _mqttPublishApp(topic, message);

        } else {

            if ((data_sensor.temperature != data_sensor_old.temperature) || ((esp_timer_get_time() - time_last_send_data_temperature) > MAX_DELAY_SEND_DATA)) {

                sprintf(message, "%d.%d", data_sensor.temperature>>8, (uint8_t)(data_sensor.temperature & 0x00FF));
                sprintf(topic, BASE_TOPIC_NAME, "temp");
                _mqttPublishApp(topic, message);
                data_sensor_old.temperature = data_sensor.temperature;
                time_last_send_data_temperature = esp_timer_get_time();

            }

            if ((data_sensor.humidity != data_sensor_old.humidity) || ((esp_timer_get_time() - time_last_send_data_humidity) > MAX_DELAY_SEND_DATA)) {

                sprintf(message, "%d.%d", data_sensor.humidity>>8, (uint8_t)(data_sensor.humidity & 0x00FF));
                sprintf(topic, BASE_TOPIC_NAME, "humidity");
                _mqttPublishApp(topic, message);
                data_sensor_old.humidity = data_sensor.humidity;
                time_last_send_data_humidity = esp_timer_get_time();

            }
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void dht11_read(data_sensor_t* data) {

    uint64_t time_start, time_stop, time_delta, time_read_begin, current_time;
    portMUX_TYPE my_spinlock = portMUX_INITIALIZER_UNLOCKED;

    data->humidity = 0;
    data->temperature = 0;
    data->crc = 0;

    gpio_set_direction(_pin_sensor, GPIO_MODE_OUTPUT);
    gpio_set_level(_pin_sensor, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(_pin_sensor, 1);
    gpio_set_direction(_pin_sensor, GPIO_MODE_INPUT);

    taskENTER_CRITICAL(&my_spinlock);

    timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &time_read_begin);
    timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &current_time);

    while (gpio_get_level(_pin_sensor) == 1 && current_time - time_read_begin < MAX_TIME_SENSOR_WAIT_US){
        timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &current_time);
    } //answer begin
    while (gpio_get_level(_pin_sensor) == 0 && current_time - time_read_begin < MAX_TIME_SENSOR_WAIT_US){
        timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &current_time);
    }; //Answer end, DHT pull up voltage
    while (gpio_get_level(_pin_sensor) == 1 && current_time - time_read_begin < MAX_TIME_SENSOR_WAIT_US){
        timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &current_time);
    }; //Data begin
    while (gpio_get_level(_pin_sensor) == 0 && current_time - time_read_begin < MAX_TIME_SENSOR_WAIT_US){
        timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &current_time);
    }; //First bit begin

    for (int i = 0; i <= 4; i+=2) {
        for (int j = 2; j > 0 ; --j) {
            for (int k = 0; k < 8; ++k) {
                timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &time_start);
                while (gpio_get_level(_pin_sensor) == 1 && current_time - time_read_begin < MAX_TIME_SENSOR_WAIT_US){
                    timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &current_time);
                };
                timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &time_stop);
                time_delta = time_stop - time_start;

                if (time_delta > 40) {
                    *(((uint8_t *) data) + i + j - 1) |= (1 << (7 - k));
                }
                if (i + j == 6) {
                    taskEXIT_CRITICAL(&my_spinlock);
                    return;
                }
                while (gpio_get_level(_pin_sensor) == 0 && current_time - time_read_begin < MAX_TIME_SENSOR_WAIT_US) {
                    timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &current_time);
                };
            }
        }
    }
    taskEXIT_CRITICAL(&my_spinlock);
}