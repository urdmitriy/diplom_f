//
// Created by urdmi on 17.12.2023.
//

#ifndef DIPLOM_ESP32_MQTT_ADAPTER_UART_DATA_H
#define DIPLOM_ESP32_MQTT_ADAPTER_UART_DATA_H
#include <stdio.h>
#include "target.h"
#define TO_DWIN 0x82
#define FROM_DWIN 0x83

#define DATA_SIZE 137

typedef void (*packet_handler)(char *rx_buffer);

typedef enum {
    DATA_TYPE_NA,
    DATA_TYPE_CMD,
	DATA_TYPE_STATE,
    DATA_TYPE_DATA,
}packet_type_e;

typedef enum {
    COMMAND_NA,
    COMMAND_ON_LOAD,
    COMMAND_OFF_LOAD,
    COMMAND_SUBSCRIBE_TOPIC,
    COMMAND_UNSUBSCRIBE_TOPIC,
}commands_e;

typedef enum {
    PARAMETR_NA,
    PARAMETR_INSOL,
    PARAMETR_INPUTS,
    PARAMETR_TEMP,
    PARAMETR_HUMIDITY,
    PARAMETR_ONPAYLOAD,
}parametr_name_e;

typedef struct uart_data {
    uint32_t data_type;
    char value[135];
    uint8_t id_parametr;
    uint8_t crc;
} uart_data_t;

#define HEADER_PACKET       0xA55A //0x5AA5 - 2b

#define ADDR_HUMIDITY       0x0050 //0x5000 - 5b
#define ADDR_INSOL          0x0550 //0x5005 - 5b
#define ADDR_MESSAGE        0x0B50 //0x500B - 100b
#define MESSAGE_LEN         100
#define ADDR_TOPIC_BASE     0x0C51 //0x510C - 10b
#define ADDR_TEMPERATURE    0x1D51 //0x511D - 5b
#define ADDR_INPUT          0x2251 //0x5122 - 4b
#define ADDR_PAYLOAD        0x2651 //0x5126 - 4b
#define ADDR_TOPIC_SUBSCRIBE 0x2A51 //0x512A - 4b
#define ADDR_PAYLOAD_BTN     0x2C51 //0x512C - 2b

typedef struct dwin_data {
    uint16_t header;
    uint8_t len;
    uint8_t direction;
    uint16_t address;
    char data[128];
}dwin_data_t;

#endif //DIPLOM_ESP32_MQTT_ADAPTER_UART_DATA_H
