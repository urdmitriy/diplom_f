//
// Created by urdmi on 17.12.2023.
//

#include "uart_esp.h"
#include "uart_data.h"
#include "driver/uart.h"
#include "string.h"
#include "esp_log.h"

static const int RX_BUF_SIZE = 1024;
static QueueHandle_t *_queue_message_to_send;

#define TXD_PIN 17
#define RXD_PIN 18

void uart_init(QueueHandle_t *queue_message_to_send){
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
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    xTaskCreate(rx_task, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(tx_task, "uart_tx_task", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);
}
int sendData(const char* data)
{
    const int len = strlen(data);
    const int txBytes = uart_write_bytes(UART_NUM_1, data, len);
    ESP_LOGI("SendData", "Wrote %d bytes", txBytes);
    return txBytes;
}
static void tx_task(void *arg)
{

    uart_data_t data_to_send;
    xQueueReceive(*_queue_message_to_send, &data_to_send, portMAX_DELAY);
    ESP_LOGI("UART", "Data ready to send!");
    sendData((char*) &data_to_send);
    vTaskDelay(pdMS_TO_TICKS(20));

}

static void rx_task(void *arg)
{
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    while (1) {
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 1000 / portTICK_PERIOD_MS);
        if (rxBytes > 0) {
            data[rxBytes] = 0;
            ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, data);
            ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);
        }
    }
    free(data);
}