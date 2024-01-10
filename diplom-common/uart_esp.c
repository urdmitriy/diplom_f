//
// Created by urdmi on 17.12.2023.
//

#include "uart_esp.h"

static QueueHandle_t _queue_message_to_send;
static uint8_t _rx_buffer[256];
packet_handler _packet_handler_app;
mqtt_publish_app _mqttPublishApp;
mqtt_subscribe_app _mqttSubscribeApp;
mqtt_unsubscribe_app _mqttUnsubscribeApp;

#define TXD_PIN 17
#define RXD_PIN 18

void uart_init(mqtt_publish_app mqttPublishApp, mqtt_subscribe_app mqttSubscribeApp, mqtt_unsubscribe_app mqttUnsubscribeApp){

    _mqttPublishApp = mqttPublishApp;
    _mqttSubscribeApp = mqttSubscribeApp;
    _mqttUnsubscribeApp = mqttUnsubscribeApp;
    _queue_message_to_send = xQueueCreate(50, sizeof(uart_data_t));

    const uart_config_t uart_config = {
            .baud_rate = 115200,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .source_clk = UART_SCLK_DEFAULT,
    };
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_1, 256, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    xTaskCreate(rx_task, "uart_rx_task", 4096, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(tx_task, "uart_tx_task", 4096, NULL, configMAX_PRIORITIES-1, NULL);
}
int sendData(const char* data)
{
    const int len = sizeof (uart_data_t); //strlen(data);
    const int txBytes = uart_write_bytes(UART_NUM_1, data, len);
    ESP_LOGI("SendData", "Wrote %d bytes", txBytes);
    return txBytes;
}
static void tx_task(void *arg)
{
    uart_data_t data_to_send;
    for (; ; ) {
        if (xQueueReceive(_queue_message_to_send, &data_to_send, pdMS_TO_TICKS(10))){
            ESP_LOGI("UART", "Data ready to send! value=%s, data_type=%lu", data_to_send.value, data_to_send.data_type);
            sendData((char*) &data_to_send);
        }
    }
}

static void rx_task(void *arg)
{
    static const char *RX_TASK_TAG = "RX_TASK";
    for (;;) {
        const int rxBytes = uart_read_bytes(UART_NUM_1, _rx_buffer, sizeof (uart_data_t), 1000 / portTICK_PERIOD_MS);
        if (rxBytes > 0) {
            _rx_buffer[rxBytes] = 0;
            ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, _rx_buffer);
            ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, _rx_buffer, rxBytes, ESP_LOG_INFO);
        }
        if (rxBytes >= sizeof (uart_data_t)){
            uart_esp_uart_data_handler((char*)_rx_buffer);
        }
    }
}

void send_status_mqtt_adapter(char * message){
    uart_data_t data_to_send;
    data_to_send.data_type = DATA_TYPE_STATE;
    memset((uint8_t*)data_to_send.value, '\0', sizeof (data_to_send.value));
    memcpy(&data_to_send.value, message, strlen(message));
    data_to_send.crc = crc8ccitt((uint8_t*)&data_to_send, DATA_SIZE);
    xQueueSend(_queue_message_to_send, &data_to_send, pdMS_TO_TICKS(10));
}

void uart_esp_uart_data_handler(char *rx_buffer){

    uart_data_t data_rcv = *(uart_data_t*)rx_buffer;

    if (data_rcv.crc != crc8ccitt(&data_rcv, DATA_SIZE) && data_rcv.crc != 0) return;
    char topic_master[128];
    switch (data_rcv.data_type) {
        case DATA_TYPE_CMD:
            switch ((commands_e)data_rcv.id_parametr) {

                case COMMAND_SUBSCRIBE_TOPIC:{
                    strcpy(topic_master, data_rcv.value);
                    _mqttSubscribeApp(topic_master);
                }
                    break;
                case COMMAND_UNSUBSCRIBE_TOPIC: {
                    strcpy(topic_master, data_rcv.value);
                    _mqttUnsubscribeApp(topic_master);
                }
                    break;
                case COMMAND_OFF_LOAD:{
                    char topic[128], message[10];
                    sprintf(topic, BASE_TOPIC_NAME, "onpayload");
                    sprintf(message, "0");
                    _mqttPublishApp(topic, message);
                }
                    break;
                case COMMAND_ON_LOAD:{
                    char topic[128], message[10];
                    sprintf(topic, BASE_TOPIC_NAME, "onpayload");
                    sprintf(message, "1");
                    _mqttPublishApp(topic, message);
                }
                    break;
                default:
                    break;
            }
            break;

        case DATA_TYPE_DATA:{
            xQueueSend(_queue_message_to_send, &data_rcv, pdMS_TO_TICKS(10));
        }
            break;
        case DATA_TYPE_STATE:
        default:
            break;
    }
}