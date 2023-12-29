//
// Created by urdmi on 25.11.2023.
//

#include "mqtt_esp.h"

static QueueHandle_t* _queue_message_to_send;
static esp_mqtt_client_handle_t* _client_to_publish;
static esp_mqtt_client_handle_t* _client_to_subscribe;

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

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI("TAG_MQTT", "MQTT_EVENT_CONNECTED");

#if defined ESP_PUBLISHER
            xTaskCreate(dht11_vTask_read, "DHT11", 4096, NULL, 1, NULL);
            xTaskCreate(di_vTask, "di_vTask", 2048, NULL, 10, NULL);
            xTaskCreate(di_vTask_periodic, "di_vTask", 2048, NULL, 10, NULL);
            xTaskCreate(light_sensor_vTask, "light_sensor_vTask", 2048, NULL, 10, NULL);
            esp_mqtt_client_subscribe(*_client_to_publish, "gb_iot/2950_UDA/onpayload", 0);
#elif defined ESP_MQTT_ADAPTER
            send_status_mqtt_adapter(STATUS_MQTT_SERVER_IS_CONNECT);
            mqtt_subscribe("2950_UDA");
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
            send_status_mqtt_adapter(STATUS_MQTT_RCV_DATA);
            char topic[100];
            char data_topic[10];

            sprintf(topic, "%.*s", event->topic_len, event->topic);
            sprintf(data_topic, "%.*s", event->data_len, event->data);

            uart_data_t data;

            data.data_type = DATA_TYPE_DATA;
            data.id_parametr = (int)get_param_name(topic);
            sprintf(data.value, "%s", data_topic);
            memset((uint8_t*)data.value, '\0', sizeof (data.value));
            data.crc = crc8ccitt((uint8_t*)&data, DATA_SIZE);

            ESP_LOGI("RES", "Data type = %d, parametr = %d, value = %s, crc = %d", data.data_type, data.id_parametr, data.value, data.crc);
            xQueueSend(*_queue_message_to_send, &data, pdMS_TO_TICKS(10));

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

void mqtt_subscribe(char* group){
    char _topic[128];
    sprintf(_topic, "gb_iot/%s/insol", group);
    esp_mqtt_client_subscribe(*_client_to_subscribe, _topic, 0);
    sprintf(_topic, "gb_iot/%s/inputs", group);
    esp_mqtt_client_subscribe(*_client_to_subscribe, _topic, 0);
    sprintf(_topic, "gb_iot/%s/temp", group);
    esp_mqtt_client_subscribe(*_client_to_subscribe, _topic, 0);
    sprintf(_topic, "gb_iot/%s/humidity", group);
    esp_mqtt_client_subscribe(*_client_to_subscribe, _topic, 0);
}

void mqtt_unsubscribe(char* group){
    char _topic[128];
    sprintf(_topic, "gb_iot/%s/insol", group);
    esp_mqtt_client_unsubscribe(*_client_to_subscribe, _topic);
    sprintf(_topic, "gb_iot/%s/inputs", group);
    esp_mqtt_client_unsubscribe(*_client_to_subscribe, _topic);
    sprintf(_topic, "gb_iot/%s/temp", group);
    esp_mqtt_client_unsubscribe(*_client_to_subscribe, _topic);
    sprintf(_topic, "gb_iot/%s/humidity", group);
    esp_mqtt_client_unsubscribe(*_client_to_subscribe, _topic);
}

#if defined ESP_PUBLISHER
void mqtt_app_start(esp_mqtt_client_handle_t* mqtt_client_publish, esp_mqtt_client_handle_t* mqtt_client_subscibe)
#elif defined ESP_MQTT_ADAPTER
void mqtt_app_start(esp_mqtt_client_handle_t* mqtt_client_publish, esp_mqtt_client_handle_t* mqtt_client_subscibe, QueueHandle_t* queue_message_to_send)
#endif
{
#if defined ESP_PUBLISHER
#elif defined ESP_MQTT_ADAPTER
    _queue_message_to_send = queue_message_to_send;
#endif
    _client_to_subscribe = mqtt_client_subscibe;
    _client_to_publish = mqtt_client_publish;

    esp_mqtt_client_config_t mqtt_cfg_publish = {
            .broker.address.uri = "mqtt://erinaceto.ru",
            .broker.address.port = 1883,
            .credentials.authentication.password = "qwope354F",
            .credentials.username = "user",
    };
    esp_mqtt_client_config_t mqtt_cfg_suscribe = {
            .broker.address.uri = "mqtt://erinaceto.ru",
            .broker.address.port = 1883,
            .credentials.authentication.certificate = "",
            .credentials.authentication.key = "",
    }; //https://github.com/espressif/esp-mqtt/issues/125?ysclid=lqqc6jdja0255111744

    *_client_to_subscribe = esp_mqtt_client_init(&mqtt_cfg_suscribe);
    *_client_to_publish = esp_mqtt_client_init(&mqtt_cfg_publish);
    esp_mqtt_client_register_event(*_client_to_subscribe, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_register_event(*_client_to_publish, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(*_client_to_subscribe);
    esp_mqtt_client_start(*_client_to_publish);
}

void uart_data_handler(char *rx_buffer){
    uart_data_t data_rcv = *(uart_data_t*)rx_buffer;

    if (data_rcv.crc != crc8ccitt(&data_rcv, DATA_SIZE)) return;

    switch (data_rcv.data_type) {
        case DATA_TYPE_CMD:
            switch ((commands_e)data_rcv.id_parametr) {

                case COMMAND_SUBSCRIBE_TOPIC:{
                    char topic_master[128];
                    strcpy(topic_master, data_rcv.value);
                    mqtt_subscribe(topic_master);
                }
                    break;
                case COMMAND_UNSUBSCRIBE_TOPIC: {
                    char topic_master[128];
                    strcpy(topic_master, data_rcv.value);
                    mqtt_unsubscribe(topic_master);
                }
                    break;
                case COMMAND_OFF_LOAD:{
                    char topic[128], message[10];
                    sprintf(topic, BASE_TOPIC_NAME, "onpayload");
                    sprintf(message, "0");
                    esp_mqtt_client_publish(*_client_to_publish, topic,  message, 0, 1, 0);
                }
                    break;
                case COMMAND_ON_LOAD:{
                    char topic[128], message[10];
                    sprintf(topic, BASE_TOPIC_NAME, "onpayload");
                    sprintf(message, "1");
                    esp_mqtt_client_publish(*_client_to_publish, topic,  message, 0, 1, 0);
                }
                    break;
                default:
                    break;
            }
            break;

        case DATA_TYPE_DATA:
        case DATA_TYPE_STATE:
        default:
            break;
    }
}