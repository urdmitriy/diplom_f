//
// Created by urdmi on 17.12.2023.
//

#ifndef DIPLOM_ESP32_MQTT_ADAPTER_UART_DATA_H
#define DIPLOM_ESP32_MQTT_ADAPTER_UART_DATA_H

typedef enum {
    DATA_TYPE_CMD,
    DATA_TYPE_DATA,
}data_type_e;

typedef enum {
    PARAMETR_NA,
    PARAMETR_INSOL,
    PARAMETR_INPUTS,
    PARAMETR_TEMP,
    PARAMETR_HUMIDITY,
}parametr_name_e;

typedef struct uart_data {
    data_type_e data_type;
    parametr_name_e parametr;
    int value;
    uint8_t crc;
} uart_data_t;

#endif //DIPLOM_ESP32_MQTT_ADAPTER_UART_DATA_H
