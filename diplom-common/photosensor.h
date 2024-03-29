//
// Created by urdmi on 16.12.2023.
//

#ifndef DIPLOM_ESP32_PHOTOSENSOR_H
#define DIPLOM_ESP32_PHOTOSENSOR_H

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "mqtt_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

typedef void (*mqtt_publish_app)(char* topic, char* message);

void light_sensor_init(adc_channel_t channel, mqtt_publish_app mqttPublishApp, SemaphoreHandle_t * semaphore_wake_up);
void light_sensor_vTask(void *arg);

#endif //DIPLOM_ESP32_PHOTOSENSOR_H
