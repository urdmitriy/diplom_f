//
// Created by urdmi on 16.12.2023.
//

#include "mqtt_client.h"

#ifndef DIPLOM_ESP32_DI_H
#define DIPLOM_ESP32_DI_H
typedef void (*mqtt_publish_app)(char* topic, char* message);
void di_init(int pin, mqtt_publish_app mqttPublishApp);
void di_vTask(void* arg);
void di_vTask_periodic(void *arg);
#endif //DIPLOM_ESP32_DI_H
