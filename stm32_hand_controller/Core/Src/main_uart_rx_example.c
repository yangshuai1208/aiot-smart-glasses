#include "hand_protocol.h"
#include <string.h>
#include <stdio.h>

#define UART_RX_BUF_SIZE  64

uint8_t uart_rx_byte=0;
char uart_line_buf[UART_RX_BUF_SIZE];
uint8_t uart_line_index=0;

HAL_UART_Receive_IT(&huart1,&uart_rx_byte,1);

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
 if(huart->Instance==USART1){
    if(uart_rx_byte=='\n')
    {
       uart_line_buf[uart_line_index] = '\0';

        if(uart_line_index>0)
        {
            hand_action_t action=hand_protocol_parse(uart_line_buf);

            printf("RX CMD:%s \r\n",uart_line_buf);
            printf("PARSE:%s\r\n",hand_action_to_string(action));

            hand_action_execute(action);
        }
        uart_line_index=0;
    }else if(uart_rx_byte!='\r'){
        if(uart_line_index<UART_RX_BUF_SIZE-1){
            uart_line_buf[uart_line_index++]=uart_rx_byte;

        }else{
            uaart_line_index=0;
            printf("RX line too long ,drop\r\n");
        }

    }
    HAL_UART_Receive_IT(&huart1,&uart_rx_byte,1);
 }   
}