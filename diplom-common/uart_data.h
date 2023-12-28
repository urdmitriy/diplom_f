//
// Created by urdmi on 17.12.2023.
//

#ifndef DIPLOM_ESP32_MQTT_ADAPTER_UART_DATA_H
#define DIPLOM_ESP32_MQTT_ADAPTER_UART_DATA_H
#include <stdio.h>

#define TO_DWIN 82
#define FROM_DWIN 83

#define DATA_SIZE 137

typedef void (*packet_handler)(char *rx_buffer);

typedef enum {
    DATA_TYPE_CMD,
	DATA_TYPE_STATE,
    DATA_TYPE_DATA,
}packet_type_e;

typedef enum {
    COMMAND_ON_LOAD,
    COMMAND_OFF_LOAD,
    COMMAND_SUBSCRIBE_TOPIC,
    COMMAND_UNSUBSCRIBE_TOPIC,
}commands_e;

typedef enum {
    STATUS_CONNECTING_WIFI,
    STATUS_WIFI_IS_CONNECT,
    STATUS_CONNECTING_MQTT_SERVER,
    STATUS_MQTT_SERVER_IS_CONNECT,
    STATUS_SUBSCRIBE_OK,
    STATUS_UNSUBSCRIBE_OK,
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
    char value[128];
    uint8_t id_parametr;
    uint8_t crc;
} uart_data_t;

#define HEADER_PACKET       0xA55A //0x5AA5
#define ADDR_TEMPERATURE    0x0051 //0x5100
#define ADDR_HUMIDITY       0x0051 //0x5100
#define ADDR_INSOL          0x0051 //0x5100
#define ADDR_INPUT          0x0051 //0x5100
#define ADDR_MESSAGE        0x0051 //0x5100

typedef struct dwin_data {
    uint16_t header;
    uint8_t len;
    uint8_t direction;
    uint16_t address;
    char data[128];
}dwin_data_t;

#endif //DIPLOM_ESP32_MQTT_ADAPTER_UART_DATA_H
