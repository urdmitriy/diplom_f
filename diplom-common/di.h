//
// Created by urdmi on 16.12.2023.
//

#include "mqtt_client.h"
#include "leds.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "inttypes.h"
#include "esp_log.h"

#ifndef DIPLOM_ESP32_DI_H
#define DIPLOM_ESP32_DI_H
typedef void (*mqtt_publish_app)(char* topic, char* message);
void di_init(int pin, mqtt_publish_app mqttPublishApp, SemaphoreHandle_t * semaphore_wake_up, int *payload_state);
void di_vTask(void* arg);
void di_wake_up_vTask(void* arg);
#endif //DIPLOM_ESP32_DI_H
