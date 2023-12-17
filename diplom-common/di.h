//
// Created by urdmi on 16.12.2023.
//

#include "mqtt_client.h"

#ifndef DIPLOM_ESP32_DI_H
#define DIPLOM_ESP32_DI_H

void di_init(int pin, esp_mqtt_client_handle_t* mqtt_client);
void di_vTask(void* arg);
void di_vTask_periodic(void *arg);
#endif //DIPLOM_ESP32_DI_H
