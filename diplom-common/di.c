//
// Created by urdmi on 16.12.2023.
//

#include "target.h"
#include "di.h"

static QueueHandle_t gpio_evt_queue = NULL;
static int _pin_di;
static mqtt_publish_app _mqttPublishApp;
static SemaphoreHandle_t *_semaphore_wake_up;
static QueueSetHandle_t queue_set;
static QueueSetMemberHandle_t member_queue_set;
static int *_payload_state;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint8_t gpio_state = (int) gpio_get_level(_pin_di);
    xQueueSendFromISR(gpio_evt_queue, &gpio_state, NULL);
}

void di_vTask(void* arg)
{
    uint8_t io_state;
    char message[10], topic[300] = {'\0',};

    for (;;) {

        member_queue_set = xQueueSelectFromSet(queue_set, pdMS_TO_TICKS(60000));

        if (member_queue_set == gpio_evt_queue) {

            xQueueReceive(gpio_evt_queue, &io_state, pdMS_TO_TICKS(10));
            vTaskDelay(pdMS_TO_TICKS(100));

        } else if (member_queue_set == *_semaphore_wake_up) {

            xSemaphoreTake(*_semaphore_wake_up, pdMS_TO_TICKS(10));

        }

        sprintf(message, "%d", io_state);
        sprintf(topic, BASE_TOPIC_NAME, "inputs");
        _mqttPublishApp(topic, message);

        sprintf(message, "%d", *_payload_state == 1 ? 1 : 0);
        sprintf(topic, BASE_TOPIC_NAME, "onpayload");
        _mqttPublishApp(topic, message);

    }
}

void di_init(int pin, mqtt_publish_app mqttPublishApp, SemaphoreHandle_t *semaphore_wake_up, int *payload_state){
    _pin_di = pin;
    _mqttPublishApp = mqttPublishApp;
    _payload_state = payload_state;
    _semaphore_wake_up = semaphore_wake_up;

    gpio_evt_queue = xQueueCreate(1, sizeof (int));
    queue_set = xQueueCreateSet(2);

    xQueueAddToSet(gpio_evt_queue, queue_set);
    xQueueAddToSet(*_semaphore_wake_up, queue_set);

    gpio_reset_pin(_pin_di);
    gpio_set_direction(_pin_di, GPIO_MODE_INPUT);
    gpio_set_pull_mode(_pin_di, GPIO_PULLDOWN_ONLY);
    gpio_set_intr_type(_pin_di, GPIO_INTR_ANYEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(_pin_di, gpio_isr_handler, NULL);
}
