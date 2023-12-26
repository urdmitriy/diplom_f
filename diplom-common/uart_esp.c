//
// Created by urdmi on 17.12.2023.
//

#include "uart_esp.h"

QueueHandle_t *_queue_message_to_send;
static uint8_t _rx_buffer[256];
packet_handler _packet_handler_app;

#define TXD_PIN 17
#define RXD_PIN 18

void uart_init(packet_handler packet_handler_app, QueueHandle_t *queue_message_to_send){
    _packet_handler_app = packet_handler_app;
    _queue_message_to_send = queue_message_to_send;

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
        if (xQueueReceive(*_queue_message_to_send, &data_to_send, pdMS_TO_TICKS(10))){
            ESP_LOGI("UART", "Data ready to send! value=%lu, data_type=%d", data_to_send.value_uint32, data_to_send.data_type);
            sendData((char*) &data_to_send);
            vTaskDelay(pdMS_TO_TICKS(10));
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
            _packet_handler_app((char*)_rx_buffer);
        }
    }
}

void send_status_mqtt_adapter(state_e status){
    uart_data_t data_to_send;
    data_to_send.data_type = DATA_TYPE_STATE;
    data_to_send.value_uint32 = 0;
    memset((uint8_t*)data_to_send.value_string, '\0', sizeof (data_to_send.value_string));
    data_to_send.id_parametr = (int)status;
    data_to_send.crc = crc8ccitt((uint8_t*)&data_to_send, DATA_SIZE);
    sendData((char*)&data_to_send);
}