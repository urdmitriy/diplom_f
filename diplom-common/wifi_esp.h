//
// Created by urdmi on 25.11.2023.
//

#ifndef DIPLOM_ESP32_WIFI_ESP_H
#define DIPLOM_ESP32_WIFI_ESP_H

#include "wifi_esp.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_log.h"
#include "freertos/event_groups.h"
#include "leds.h"
#include "uart_esp.h"
#include "mqtt_esp.h"

typedef void (*mqtt_start_app)(void);

void wifi_init_sta(char* essid, char* wifipass, mqtt_start_app mqttStartApp);

#endif //DIPLOM_ESP32_WIFI_ESP_H
