//
// Created by urdmi on 25.11.2023.
//

#include "dht11.h"

#define MAX_TIME_SENSOR_WAIT_US 10000

static int _pin_sensor_power, _pin_sensor_data;
static mqtt_publish_app _mqttPublishApp;
static SemaphoreHandle_t *_semaphore_wake_up;

void dht11_init(int pin_sensor_power, int pin_sensor_data, mqtt_publish_app mqttPublishApp, SemaphoreHandle_t * semaphore_wake_up) {
    _pin_sensor_power = pin_sensor_power;
    _pin_sensor_data = pin_sensor_data;
    _mqttPublishApp = mqttPublishApp;
    _semaphore_wake_up = semaphore_wake_up;

    gpio_reset_pin(_pin_sensor_power);
    gpio_reset_pin(_pin_sensor_data);

    gpio_set_direction(_pin_sensor_power, GPIO_MODE_OUTPUT);


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
    char message[500], topic[300];

    for( ;; )
    {
        if (xSemaphoreTake(*_semaphore_wake_up, pdMS_TO_TICKS(5000)) == pdFALSE){
            //если сработка по таймауту, делаем опрос и отправку

            gpio_set_level(_pin_sensor_power, 1);
            vTaskDelay(pdMS_TO_TICKS(1000));
            dht11_read(&data_sensor);
            gpio_set_level(_pin_sensor_power, 0);

            uint8_t crc_calc =
                    (uint8_t)(data_sensor.temperature>>8)+
                    (uint8_t)(data_sensor.temperature & 0x00FF) +
                    (uint8_t)(data_sensor.humidity>>8);
            ESP_LOGI("DHT11", "Temperature: %d.%d, humidity: %d.%d, crc: %d, crc calc: %d.",
                     data_sensor.temperature>>8, data_sensor.temperature & 0x00FF, data_sensor.humidity>>8,
                     data_sensor.humidity & 0x00FF, data_sensor.crc, crc_calc);

            if (data_sensor.crc == crc_calc) {

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

        } else {
            //если сработка по семафору, отправляем последние данные
            sprintf(message, "%d.%d", data_sensor.temperature>>8, (uint8_t)(data_sensor.temperature & 0x00FF));
            sprintf(topic, BASE_TOPIC_NAME, "temp");
            _mqttPublishApp(topic, message);

            sprintf(message, "%d.%d", data_sensor.humidity>>8, (uint8_t)(data_sensor.humidity & 0x00FF));
            sprintf(topic, BASE_TOPIC_NAME, "humidity");
            _mqttPublishApp(topic, message);
        }
    }
}

void dht11_read(data_sensor_t* data) {

    uint64_t time_start, time_stop, time_delta, time_read_begin, current_time;
    portMUX_TYPE my_spinlock = portMUX_INITIALIZER_UNLOCKED;

    data->humidity = 0;
    data->temperature = 0;
    data->crc = 0;

    gpio_set_direction(_pin_sensor_data, GPIO_MODE_OUTPUT);
    gpio_set_level(_pin_sensor_data, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(_pin_sensor_data, 1);
    gpio_set_direction(_pin_sensor_data, GPIO_MODE_INPUT);

    timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &time_read_begin);
    timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &current_time);

    taskENTER_CRITICAL(&my_spinlock);

    while (gpio_get_level(_pin_sensor_data) == 1 && current_time - time_read_begin < MAX_TIME_SENSOR_WAIT_US){
        timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &current_time);
    } //answer begin
    while (gpio_get_level(_pin_sensor_data) == 0 && current_time - time_read_begin < MAX_TIME_SENSOR_WAIT_US){
        timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &current_time);
    }; //Answer end, DHT pull up voltage
    while (gpio_get_level(_pin_sensor_data) == 1 && current_time - time_read_begin < MAX_TIME_SENSOR_WAIT_US){
        timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &current_time);
    }; //Data begin
    while (gpio_get_level(_pin_sensor_data) == 0 && current_time - time_read_begin < MAX_TIME_SENSOR_WAIT_US){
        timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &current_time);
    }; //First bit begin

    for (int i = 0; i <= 4; i+=2) {
        for (int j = 2; j > 0 ; --j) {
            for (int k = 0; k < 8; ++k) {
                timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &time_start);
                while (gpio_get_level(_pin_sensor_data) == 1 && current_time - time_read_begin < MAX_TIME_SENSOR_WAIT_US){
                    timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &current_time);
                };
                timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &time_stop);
                time_delta = time_stop - time_start;
                if (i + j == 6) j--;
                if (time_delta > 40) {
                    *(((uint8_t *) data) + i + j - 1) |= (1 << (7 - k));
                }

                while (gpio_get_level(_pin_sensor_data) == 0 && current_time - time_read_begin < MAX_TIME_SENSOR_WAIT_US) {
                    timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &current_time);
                };
            }

        }
    }
    taskEXIT_CRITICAL(&my_spinlock);
}