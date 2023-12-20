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

void uart_init(packet_handler packet_handler_app, QueueHandle_t *queue_message_to_send);
int sendData(const char* data);
static void tx_task(void *arg);
static void rx_task(void *arg);
void send_status_mqtt_adapter(state_e status);

#endif //DIPLOM_ESP32_MQTT_ADAPTER_UART_ESP_H
