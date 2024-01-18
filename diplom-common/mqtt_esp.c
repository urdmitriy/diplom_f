//
// Created by urdmi on 25.11.2023.
//

#include "mqtt_esp.h"
#define YANDEX
//#define ERINACETO

static esp_mqtt_client_handle_t mqtt_client_publish;
static esp_mqtt_client_handle_t mqtt_client_subscibe;
static app_after_mqtt_connected _appAfterMqttConnected;
static char* _root_ca;
static char* _cert_devices;
static char* _key_devices;
static char* _cert_registr;
static char* _key_registr;
static char _current_topic[10] = {'\0',};
static app_after_mqtt_rcv_data _appMqttRcvData;


void mqtt_esp_init(char* root_ca, char* cert_devices, char* key_devices, char* cert_registr, char* key_registr,
                   app_after_mqtt_connected appAfterMqttConnected,
                   app_after_mqtt_rcv_data appMqttRcvData){

    _root_ca = root_ca;
    _cert_devices = cert_devices;
    _key_devices = key_devices;
    _cert_registr = cert_registr;
    _key_registr = key_registr;
    _appAfterMqttConnected = appAfterMqttConnected;
    _appMqttRcvData = appMqttRcvData;

}

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE("TAG_MQTT", "Last error %s: 0x%x", message, error_code);
    }
}

parametr_name_e mqtt_esp_get_param_name(char *topic_name) {
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
    strcpy(my_topic, "gb_iot/2950_UDA/onpayload");
    if (strcmp(topic_name, my_topic) == 0) {
        return PARAMETR_ONPAYLOAD;
    }
    ESP_LOGI("DATA_TYPE", "Incom data topic: NA: %s", topic_name);
    return PARAMETR_NA;
}

static void mqtt_esp_mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD("TAG_MQTT", "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED: {

            esp_mqtt_client_handle_t *_client = (esp_mqtt_client_handle_t *) handler_args;
            _appAfterMqttConnected(_client, &mqtt_client_publish, &mqtt_client_subscibe);

        }
            break;

        case MQTT_EVENT_DISCONNECTED:
#ifdef ESP_MQTT_ADAPTER
            send_status_mqtt_adapter("MQTT servet is disconnect");
#endif
            ESP_LOGI("TAG_MQTT", "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
#ifdef ESP_MQTT_ADAPTER
            send_status_mqtt_adapter("MQTT subscribed");
#endif
            ESP_LOGI("TAG_MQTT", "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
#ifdef ESP_MQTT_ADAPTER
            send_status_mqtt_adapter("MQTT unsubscribed");
#endif
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
            _appMqttRcvData(event);
            break;
        }

        case MQTT_EVENT_ERROR:
            ESP_LOGI("TAG_MQTT", "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
#ifdef ESP_MQTT_ADAPTER
                send_status_mqtt_adapter("MQTT transport error");
#endif
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

void mqtt_esp_mqtt_subscribe(char* group){

    if (strlen(_current_topic) > 3) {
        mqtt_esp_mqtt_unsubscribe(_current_topic);
    }

    char _topic[128];
    sprintf(_topic, "gb_iot/%s/insol", group);
    esp_mqtt_client_subscribe(mqtt_client_subscibe, _topic, 0);
    sprintf(_topic, "gb_iot/%s/inputs", group);
    esp_mqtt_client_subscribe(mqtt_client_subscibe, _topic, 0);
    sprintf(_topic, "gb_iot/%s/temp", group);
    esp_mqtt_client_subscribe(mqtt_client_subscibe, _topic, 0);
    sprintf(_topic, "gb_iot/%s/humidity", group);
    esp_mqtt_client_subscribe(mqtt_client_subscibe, _topic, 0);
    sprintf(_topic, "gb_iot/%s/onpayload", group);
    esp_mqtt_client_subscribe(mqtt_client_subscibe, _topic, 0);

    vTaskDelay(pdMS_TO_TICKS(1000));
    sprintf(_topic, "gb_iot/%s/wake_up", group);
    mqtt_esp_publish_message(_topic, "1");

    sprintf(_current_topic, "%s", group);
}

void mqtt_esp_mqtt_unsubscribe(char* group){
    char _topic[128];
    sprintf(_topic, "gb_iot/%s/insol", group);
    esp_mqtt_client_unsubscribe(mqtt_client_subscibe, _topic);
    sprintf(_topic, "gb_iot/%s/inputs", group);
    esp_mqtt_client_unsubscribe(mqtt_client_subscibe, _topic);
    sprintf(_topic, "gb_iot/%s/temp", group);
    esp_mqtt_client_unsubscribe(mqtt_client_subscibe, _topic);
    sprintf(_topic, "gb_iot/%s/humidity", group);
    esp_mqtt_client_unsubscribe(mqtt_client_subscibe, _topic);
    sprintf(_topic, "gb_iot/%s/onpayload", group);
    esp_mqtt_client_unsubscribe(mqtt_client_subscibe, _topic);

    sprintf(_current_topic, "%s", "none");
}

void mqtt_esp_mqtt_app_start(void ){
#ifdef ERINACETO
    esp_mqtt_client_config_t mqtt_cfg_publish = {
            .broker.address.uri = "mqtt://erinaceto.ru",
            .broker.address.port = 1883,
            .credentials.client_id = "ESP_publish",
            .credentials.authentication.password = "qwope354F",
            .credentials.username = "user",
    };

    esp_mqtt_client_config_t mqtt_cfg_subscribe = {
            .broker.address.uri = "mqtt://erinaceto.ru",
            .broker.address.port = 1883,
            .credentials.client_id = "ESP_subscribe",
            .credentials.authentication.password = "qwope354F",
            .credentials.username = "user",
    };
#elif defined YANDEX  //https://github.com/espressif/esp-mqtt/issues/125?ysclid=lqqc6jdja0255111744
    ESP_ERROR_CHECK(esp_tls_init_global_ca_store());
    ESP_ERROR_CHECK(esp_tls_set_global_ca_store(
            (const unsigned char *) _root_ca,
            strlen(_root_ca) + 1));

    esp_mqtt_client_config_t mqtt_cfg_publish = {
            .broker.address.uri = "mqtts://mqtt.cloud.yandex.net",
            .broker.address.port = 8883,
            .credentials.client_id = "ESP_publish",
            .credentials.authentication.certificate = _cert_devices,
            .credentials.authentication.certificate_len = strlen(_cert_devices) + 1,
            .credentials.authentication.key = _key_devices,
            .credentials.authentication.key_len = strlen(_key_devices) + 1,
            .broker.verification.use_global_ca_store = true,
    };

    esp_mqtt_client_config_t mqtt_cfg_subscribe = {
            .broker.address.uri = "mqtts://mqtt.cloud.yandex.net",
            .broker.address.port = 8883,
            .credentials.client_id = "ESP_subscribe",
            .credentials.authentication.certificate = _cert_registr,
            .credentials.authentication.certificate_len = strlen(_cert_registr) + 1,
            .credentials.authentication.key = _key_registr,
            .credentials.authentication.key_len = strlen(_key_registr) + 1,
            .broker.verification.use_global_ca_store = true,
    };

#endif
    mqtt_client_subscibe = esp_mqtt_client_init(&mqtt_cfg_subscribe);
    mqtt_client_publish = esp_mqtt_client_init(&mqtt_cfg_publish);

    esp_mqtt_client_register_event(mqtt_client_subscibe, ESP_EVENT_ANY_ID, mqtt_esp_mqtt_event_handler, &mqtt_client_subscibe);
    esp_mqtt_client_register_event(mqtt_client_publish, ESP_EVENT_ANY_ID, mqtt_esp_mqtt_event_handler, &mqtt_client_publish);

    esp_mqtt_client_start(mqtt_client_publish); //    subscribe коннектится после коннекта издателя
}

void mqtt_esp_publish_message(char* topic, char* message){
    esp_mqtt_client_publish(mqtt_client_publish, topic, message, 0, 1, 0);
}