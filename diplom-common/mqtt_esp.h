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
#include "uart_esp.h"

typedef void (*uart_data_handler)(char *rx_buffer);
typedef void (*wake_up_action)(void);
typedef void (*app_after_mqtt_connected)(esp_mqtt_client_handle_t *active_client,
        esp_mqtt_client_handle_t *publish_client, esp_mqtt_client_handle_t *subscribe_client);
typedef void (*app_after_mqtt_rcv_data)(esp_mqtt_event_handle_t event);

void mqtt_esp_init(char* root_ca, char* cert_devices, char* key_devices, char* cert_registr, char* key_registr,
                   app_after_mqtt_connected, app_after_mqtt_rcv_data);

void mqtt_esp_mqtt_app_start();
parametr_name_e mqtt_esp_get_param_name(char *topic_name);
void mqtt_esp_mqtt_subscribe(char* group);
void mqtt_esp_mqtt_unsubscribe(char* group);
void mqtt_esp_publish_message(char* topic, char* message);
#endif //DIPLOM_ESP32_MQTT_ESP_H
