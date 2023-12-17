//
// Created by urdmi on 17.12.2023.
//

#ifndef DIPLOM_ESP32_MQTT_ADAPTER_UART_ESP_H
#define DIPLOM_ESP32_MQTT_ADAPTER_UART_ESP_H

void uart_init(void);
int sendData(const char* data);
static void tx_task(void *arg);
static void rx_task(void *arg);


#endif //DIPLOM_ESP32_MQTT_ADAPTER_UART_ESP_H
