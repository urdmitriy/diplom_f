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
#include "crc8.h"

#include "esp_tls.h"
#if defined ESP_PUBLISHER
#include "dht11.h"
#include "photosensor.h"
#elif defined ESP_MQTT_ADAPTER
#include "uart_esp.h"
#endif
typedef void (*uart_data_handler)(char *rx_buffer);
void mqtt_esp_init(char* root_ca, char* cert_devices, char* key_devices, char* cert_registr, char* key_registr, uart_data_handler uartDataHandler);
void mqtt_esp_mqtt_app_start();
parametr_name_e mqtt_esp_get_param_name(char *topic_name);
void mqtt_esp_mqtt_subscribe(char* group);
void mqtt_esp_mqtt_unsubscribe(char* group);
void mqtt_esp_publish_message(char* topic, char* message);
#endif //DIPLOM_ESP32_MQTT_ESP_H
