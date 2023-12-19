//
// Created by urdmi on 25.11.2023.
//

#include "target.h"
#include "mqtt_esp.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include "leds.h"
#include "driver/gpio.h"
#include "di.h"

#if defined ESP_PUBLISHER
#include "dht11.h"
#include "photosensor.h"
#elif defined ESP_MQTT_ADAPTER
#include "uart_esp.h"
#include "uart_data.h"
#include "crc8.h"
#endif

static uart_data_t data;
static QueueHandle_t queue_message_to_send = NULL;

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE("TAG_MQTT", "Last error %s: 0x%x", message, error_code);
    }
}

static parametr_name_e get_param_name(char *topic_name) {
    char my_topic[100];
    strcpy(my_topic, "gb_iot/2950_UDA/insol");
    if (strcmp(topic_name, my_topic) == 0) {
        return PARAMETR_INSOL;
    }

    strcpy(my_topic, "gb_iot/2950_UDA/inputs");
    if (strcmp(topic_name, my_topic) == 0) {
        return PARAMETR_INPUTS;
    }

    strcpy(my_topic, "gb_iot/2950_UDA/temp");
    if (strcmp(topic_name, my_topic) == 0) {
        return PARAMETR_TEMP;
    }

    strcpy(my_topic, "gb_iot/2950_UDA/humidity");
    if (strcmp(topic_name, my_topic) == 0) {
        return PARAMETR_HUMIDITY;
    }

    ESP_LOGI("DATA_TYPE", "Incom data topic: NA: %s", topic_name);
    return PARAMETR_NA;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD("TAG_MQTT", "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI("TAG_MQTT", "MQTT_EVENT_CONNECTED");

#if defined ESP_PUBLISHER
            xTaskCreate(dht11_vTask_read, "DHT11", 4096, NULL, 1, NULL);
            xTaskCreate(di_vTask, "di_vTask", 2048, NULL, 10, NULL);
            xTaskCreate(di_vTask_periodic, "di_vTask", 2048, NULL, 10, NULL);
            xTaskCreate(light_sensor_vTask, "light_sensor_vTask", 2048, NULL, 10, NULL);
            esp_mqtt_client_subscribe(client, "gb_iot/2950_UDA/onpayload", 0);
#elif defined ESP_MQTT_ADAPTER
            esp_mqtt_client_subscribe(client, "gb_iot/2950_UDA/insol", 0);
            esp_mqtt_client_subscribe(client, "gb_iot/2950_UDA/inputs", 0);
            esp_mqtt_client_subscribe(client, "gb_iot/2950_UDA/temp", 0);
            esp_mqtt_client_subscribe(client, "gb_iot/2950_UDA/humidity", 0);
            uart_init(&queue_message_to_send);
#endif
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI("TAG_MQTT", "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI("TAG_MQTT", "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI("TAG_MQTT", "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI("TAG_MQTT", "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
#if defined ESP_PUBLISHER
            leds_flash(LED_YELLOW, 50);
#elif defined ESP_MQTT_ADAPTER
            leds_flash(LED_GREEN, 50);
#endif
            break;

        case MQTT_EVENT_DATA:{
            ESP_LOGI("TAG_MQTT", "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            leds_flash(LED_GREEN, 100);

            char incom_topic[50] = {'\0',};
            sprintf(incom_topic, "%.*s", event->topic_len, event->topic);

#if defined ESP_PUBLISHER

            char my_topic[] = "gb_iot/2950_UDA/onpayload";
            if (strcmp(incom_topic, my_topic) == 0) {
                if (event->data[0] == '1') {
                    gpio_set_level(LED_WHITE, 1);
                } else if (event->data[0] == '0'){
                    gpio_set_level(LED_WHITE, 0);
                }
            }
#elif defined ESP_MQTT_ADAPTER

            char topic[100];
            char data_topic[10];

            sprintf(topic, "%.*s", event->topic_len, event->topic);
            sprintf(data_topic, "%.*s", event->data_len, event->data);

            data.data_type = DATA_TYPE_DATA;
            data.id_parametr = (int)get_param_name(topic);
            float value_float = strtof(data_topic, NULL);
            data.value = (int)(value_float*10);

            data.crc = crc8ccitt((uint8_t*)&data, 9);

            ESP_LOGI("RES", "Data type = %d, parametr = %d, value = %lu, crc = %d", data.data_type, data.id_parametr, data.value, data.crc);
            xQueueSend(queue_message_to_send, &data, pdMS_TO_TICKS(10));

#endif

            break;
        }


        case MQTT_EVENT_ERROR:
            ESP_LOGI("TAG_MQTT", "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
                log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
                log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
                ESP_LOGI("TAG_MQTT", "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
            }
            break;

        default:
            ESP_LOGI("TAG_MQTT", "Other event id:%d", event->event_id);
            break;
    }
}

void mqtt_app_start(esp_mqtt_client_handle_t* _mqtt_client)
{
    queue_message_to_send = xQueueCreate(50, sizeof(uart_data_t));

    esp_mqtt_client_config_t mqtt_cfg = {
            .broker.address.uri = "mqtt://erinaceto.ru",
            .broker.address.port = 1883,
            .credentials.authentication.password = "qwope354F",
            .credentials.username = "user",
    };

    *_mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(*_mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(*_mqtt_client);
}
