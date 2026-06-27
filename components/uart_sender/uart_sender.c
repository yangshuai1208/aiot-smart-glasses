#include "uart_sender.h"

#include <string.h>

#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define UART_SENDER_PORT    UART_NUM_1
#define UART_SENDER_TX_GPIO GPIO_NUM_17
#define UART_SENDER_RX_GPIO GPIO_NUM_18
#define UART_SENDER_BAUD_RATE 115200
#define UART_SENDER_BUF_SIZE   1024

static const char*TAG="UART_SENDER";

esp_err_t uart_sender_init(void)
{
    uart_config_t uart_config={
        .baud_rate=UART_SENDER_BAUD_RATE,
        .data_bits=UART_DATA_8_BITS,
        .parity=UART_PARITY_DISABLE,
        .stop_bits=UART_STOP_BITS_1,
        .flow_ctrl=UART_HW_FLOWCTRL_DISABLE,
        .source_clk=UART_SCLK_DEFAULT,
    };
    esp_err_t ret=uart_driver_install(UART_SENDER_PORT,
                                    UART_SENDER_BUF_SIZE,
                                    0,
                                    0,
                                    NULL,
                                    0);
    if(ret!=ESP_OK){
        ESP_LOGE(TAG,"uart_driver_install failed");
        return ret;

    }

    ret=uart_param_config(UART_SENDER_PORT,&uart_config);
    if(ret!=ESP_OK){
        ESP_LOGE(TAG,"uart_param_config failed");
        return ret;
    }
    ret=uart_set_pin(UART_SENDER_PORT,
                    UART_SENDER_TX_GPIO,
                    UART_SENDER_RX_GPIO,
                    UART_PIN_NO_CHANGE,
                    UART_PIN_NO_CHANGE);
    if(ret!=ESP_OK){
        ESP_LOGE(TAG,"uart_set_pin failed");
        return ret;
    }
    ESP_LOGI(TAG,"UART sender init success,TX=GPIO17 RX=GPIO18 Baud=115200");

    return ESP_OK;

}
esp_err_t uart_sender_send_json(const char*json)
{
    if(json==NULL){
        return ESP_ERR_INVALID_ARG;

    }
    int len=strlen(json);

    int written=uart_write_bytes(UART_SENDER_PORT,json,len);
    if(written<0)
    {
        return ESP_FAIL;
    }
    uart_write_bytes(UART_SENDER_PORT,"\n",1);


    return ESP_OK;
}