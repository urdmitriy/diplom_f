//
// Created by urdmi on 25.11.2023.
//

#ifndef DIPLOM_ESP32_MQTT_ESP_H
#define DIPLOM_ESP32_MQTT_ESP_H

#include "mqtt_client.h"
#include "uart_data.h"

void mqtt_app_start(esp_mqtt_client_handle_t* mqtt_client);
static parametr_name_e get_param_name(char *topic_name);
void mqtt_subscribe(char* group);
void mqtt_unsubscribe(char* group);
void uart_data_handler(void );
#endif //DIPLOM_ESP32_MQTT_ESP_H
