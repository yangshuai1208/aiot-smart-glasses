#ifndef STM32_SENDER_H
#define STM32_SENDER_H

int stm32_sender_open(const char*device);
int stm32_sender_send_cmd(int fd,const char *cmd);
void stm32_sender_close(int fd);

#endif