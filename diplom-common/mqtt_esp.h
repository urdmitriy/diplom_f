//
// Created by urdmi on 25.11.2023.
//

#ifndef DIPLOM_ESP32_MQTT_ESP_H
#define DIPLOM_ESP32_MQTT_ESP_H

#include "target.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include "leds.h"
#include "driver/gpio.h"
#include "di.h"
#include "uart_data.h"

#if defined ESP_PUBLISHER
#include "dht11.h"
#include "photosensor.h"
#elif defined ESP_MQTT_ADAPTER
#include "uart_esp.h"
#include "crc8.h"
#endif

void mqtt_app_start(esp_mqtt_client_handle_t* mqtt_client);
static parametr_name_e get_param_name(char *topic_name);
void mqtt_subscribe(char* group);
void mqtt_unsubscribe(char* group);
void uart_data_handler(char *rx_buffer);
#endif //DIPLOM_ESP32_MQTT_ESP_H
