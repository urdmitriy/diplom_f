//
// Created by urdmi on 17.12.2023.
//

#ifndef DIPLOM_ESP32_MQTT_ADAPTER_UART_DATA_H
#define DIPLOM_ESP32_MQTT_ADAPTER_UART_DATA_H

#define DATA_SIZE 137

typedef void (*packet_handler)(void);

typedef enum {
    DATA_TYPE_CMD,
	DATA_TYPE_STATE,
    DATA_TYPE_DATA,
}packet_type_e;

typedef enum {
    COMMAND_ON_LOAD,
    COMMAND_OFF_LOAD,
    COMMAND_SUBSCRIBLE_TOPIC,
    COMMAND_UNSUBSCRIBLE_TOPIC,
}commands_e;

typedef enum {
    STATUS_CONNECTING_WIFI,
    STATUS_WIFI_IS_CONNECT,
    STATUS_CONNECTING_MQTT_SERVER,
    STATUS_MQTT_SERVER_IS_CONNECT,
    STATUS_SUBSCRIBLE_OK,
    STATUS_UNSUBSCRIBLE_OK,
    STATUS_MQTT_RCV_DATA,
    STATUS_MQTT_DISCONNECTED,
}state_e;

typedef enum {
    PARAMETR_NA,
    PARAMETR_INSOL,
    PARAMETR_INPUTS,
    PARAMETR_TEMP,
    PARAMETR_HUMIDITY,
}parametr_name_e;

typedef struct uart_data {
    packet_type_e data_type;
    uint32_t value_uint32;
    char value_string[128];
    uint8_t id_parametr;
    uint8_t crc;
} uart_data_t;

#endif //DIPLOM_ESP32_MQTT_ADAPTER_UART_DATA_H
