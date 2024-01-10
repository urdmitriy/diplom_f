//
// Created by urdmi on 17.12.2023.
//

#ifndef DIPLOM_ESP32_MQTT_ADAPTER_UART_ESP_H
#define DIPLOM_ESP32_MQTT_ADAPTER_UART_ESP_H
#include "uart_data.h"
#include "driver/uart.h"
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "crc8.h"

typedef void (*mqtt_publish_app)(char* topic, char* message);
typedef void (*mqtt_subscribe_app)(char* topic);
typedef void (*mqtt_unsubscribe_app)(char* topic);

void uart_init(mqtt_publish_app mqttPublishApp, mqtt_subscribe_app mqttSubscribeApp,
               mqtt_unsubscribe_app mqttUnsubscribeApp);
int sendData(const char* data);
static void tx_task(void *arg);
static void rx_task(void *arg);
void send_status_mqtt_adapter(char * message);
void uart_esp_uart_data_handler(char *rx_buffer);

#endif //DIPLOM_ESP32_MQTT_ADAPTER_UART_ESP_H
