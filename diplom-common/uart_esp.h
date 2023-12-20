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

void uart_init(QueueHandle_t *queue_message_to_send, uint8_t *rx_buffer, packet_handler packet_handler_app);
int sendData(const char* data);
static void tx_task(void *arg);
static void rx_task(void *arg);


#endif //DIPLOM_ESP32_MQTT_ADAPTER_UART_ESP_H
