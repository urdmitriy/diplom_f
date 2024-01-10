//
// Created by urdmi on 25.11.2023.
//

#ifndef DIPLOM_ESP32_DHT11_H
#define DIPLOM_ESP32_DHT11_H

#include <stdio.h>
#include "target.h"
#include "mqtt_esp.h"
#include "driver/gpio.h"
#include "driver/timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/timers.h"

typedef void (*mqtt_publish_app)(char* topic, char* message);

typedef struct {
    uint16_t humidity;
    uint16_t temperature;
    uint8_t crc;
} data_sensor_t;

void dht11_vTask_read(void * pvParameters );
void dht11_init(int pin_sensor, mqtt_publish_app mqttPublishApp);
void dht11_read(data_sensor_t *data);


#endif //DIPLOM_ESP32_DHT11_H
