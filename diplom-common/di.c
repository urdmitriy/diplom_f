//
// Created by urdmi on 16.12.2023.
//

#include "target.h"
#include "di.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "inttypes.h"
#include "esp_log.h"

static esp_mqtt_client_handle_t* _mqtt_client;
static QueueHandle_t gpio_evt_queue = NULL;
static int _pin_di;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint8_t gpio_state = (int) gpio_get_level(_pin_di);
    xQueueSendFromISR(gpio_evt_queue, &gpio_state, NULL);
}

void di_vTask(void* arg)
{
    uint8_t io_state;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_state, portMAX_DELAY)) {
            char message[10], topic[300] = {'\0',};
            sprintf(message, "%d", io_state);
            sprintf(topic, BASE_TOPIC_NAME, "inputs");
            esp_mqtt_client_publish(*_mqtt_client, topic,  message, 0, 1, 0);
        }
    }
}

void di_vTask_periodic(void *arg) {
    uint8_t io_state;
    for (;;) {
        io_state = gpio_get_level(_pin_di);
        char message[10], topic[300] = {'\0',};
        sprintf(message, "%d", io_state);
        sprintf(topic, BASE_TOPIC_NAME, "inputs");
        esp_mqtt_client_publish(*_mqtt_client, topic, message, 0, 1, 0);
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

void di_init(int pin, esp_mqtt_client_handle_t* mqtt_client){
    _pin_di = pin;
    _mqtt_client = mqtt_client;
    gpio_reset_pin(_pin_di);
    gpio_set_direction(_pin_di, GPIO_MODE_INPUT);
    gpio_set_pull_mode(_pin_di, GPIO_PULLDOWN_ONLY);
    gpio_set_intr_type(_pin_di, GPIO_INTR_ANYEDGE);
    gpio_evt_queue = xQueueCreate(10, sizeof(uint8_t));
    gpio_install_isr_service(0);
    gpio_isr_handler_add(_pin_di, gpio_isr_handler, NULL);
}