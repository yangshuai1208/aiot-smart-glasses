/*
 * Day20 STM32 UART RX Example
 *
 * 注意：
 * 这个文件不是完整 main.c，不能直接烧录。
 * 它用于说明：应该把下面几段代码插入到 CubeMX 生成的 main.c 对应 USER CODE 区域。
 */

/* USER CODE BEGIN Includes */
#include "hand_protocol.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
/* USER CODE END Includes */


/* USER CODE BEGIN PV */
#define UART_RX_BUF_SIZE 64

uint8_t uart_rx_byte = 0;
char uart_line_buf[UART_RX_BUF_SIZE];
uint8_t uart_line_index = 0;
/* USER CODE END PV */


/*
 * 放在 main() 中 MX_USART1_UART_Init(); 后面：
 *
 * MX_GPIO_Init();
 * MX_USART1_UART_Init();
 *
 * HAL_UART_Receive_IT(&huart1, &uart_rx_byte, 1);
 */


/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        if (uart_rx_byte == '\n') {
            uart_line_buf[uart_line_index] = '\0';

            if (uart_line_index > 0) {
                hand_action_t action = hand_protocol_parse(uart_line_buf);

                printf("RX CMD: %s\r\n", uart_line_buf);
                printf("PARSE : %s\r\n", hand_action_to_string(action));

                hand_action_execute(action);
            }

            uart_line_index = 0;
        } else if (uart_rx_byte != '\r') {
            if (uart_line_index < UART_RX_BUF_SIZE - 1) {
                uart_line_buf[uart_line_index++] = uart_rx_byte;
            } else {
                uart_line_index = 0;
                printf("RX line too long, drop\r\n");
            }
        }

        HAL_UART_Receive_IT(&huart1, &uart_rx_byte, 1);
    }
}
/* USER CODE END 4 */