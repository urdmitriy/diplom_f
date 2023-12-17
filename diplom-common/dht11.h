//
// Created by urdmi on 25.11.2023.
//

#ifndef DIPLOM_ESP32_DHT11_H
#define DIPLOM_ESP32_DHT11_H

#include "freertos/timers.h"

typedef struct {
    uint16_t humidity;
    uint16_t temperature;
    uint8_t crc;
} data_sensor_t;

void dht11_vTask_read(void * pvParameters );
void dht11_init(int pin_sensor, esp_mqtt_client_handle_t* mqtt_client);
void dht11_read(data_sensor_t *data);


#endif //DIPLOM_ESP32_DHT11_H
