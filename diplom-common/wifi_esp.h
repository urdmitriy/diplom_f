//
// Created by urdmi on 25.11.2023.
//

#ifndef DIPLOM_ESP32_WIFI_ESP_H
#define DIPLOM_ESP32_WIFI_ESP_H

#include "wifi_esp.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_log.h"
#include "freertos/event_groups.h"
#include "leds.h"
#include "uart_esp.h"
#include "mqtt_esp.h"

#if defined ESP_PUBLISHER
void wifi_init_sta(esp_mqtt_client_handle_t* client_publish, esp_mqtt_client_handle_t* client_subscribe);
#elif defined ESP_MQTT_ADAPTER
void wifi_init_sta(QueueHandle_t* queue_message_to_send, esp_mqtt_client_handle_t* client_publish, esp_mqtt_client_handle_t* client_subscribe);
#endif


#endif //DIPLOM_ESP32_WIFI_ESP_H
