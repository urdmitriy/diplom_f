//
// Created by d.uryupin on 19.12.2023.
//

#ifndef DIPLOM_ESP32_MQTT_ADAPTER_CRC8_H
#define DIPLOM_ESP32_MQTT_ADAPTER_CRC8_H

#include <stdint.h>
#include <string.h>

uint8_t crc8ccitt(const void * data, size_t size);

#endif //DIPLOM_ESP32_MQTT_ADAPTER_CRC8_H
