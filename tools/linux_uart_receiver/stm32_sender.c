#include "stm32_sender.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

static int uart_config_115200_8n1(int fd)
{
    struct termios tty;
    memset(&tty,0,sizeof(tty));

    if(tcgetattr(fd,&tty)!=0)
    {
        perror("tcgetattr stm32 uart failed");
        return -1;
    }

    cfsetispeed(&tty,B115200);
    cfsetospeed(&tty,B115200);

    tty.c_cflag &=~PARENB;
    tty.c_cflag &=~CSTOPB;
    tty.c_cflag &=~CSIZE;
    tty.c_cflag |=~CS8;
    tty.c_cflag &=~CRTSCTS;
    tty.c_cflag |=~CREAD |CLOCAL;

    tty.c_cflag &=~(IXON |IXOFF |IXANY);
    tty.c_lflag &=~(ICRNL|INLCR);

    tty.c_oflag&=~OPOST;

    tty.c_cc[VMIN]=1;
    tty.c_cc[VTIME]=0;

    tcflush(fd,TCIOFLUSH);

    if(tcsetattr(fd,TCSANOW,&tty)!=0)
    {
        perror("tcsetattr stm32 uart failed");
        return -1;
    }
    return 0;
}
int stm32_sender_open(const char*device)
{
    if(device==NULL)
    {
        return -1;
    }
    int fd=open(device,O_RDWR|O_NOCTTY);
    if(fd<0)
    {
        perror("open stm32 uart failed");
        return -1;
    }
    if(uart_config_115200_8n1(fd)!=0)
    {
        close(fd);
        return -1;
    }
    printf("STM32 sender opened:%s\n",device);
    return fd;
}
int stm32_sender_send_cmd(int fd,const char*cmd)
{
    if(fd<0||cmd==NULL)
    {
        return -1;
    }
    int len=strlen(cmd);

    if(write(fd,cmd,len)!=len)
    {
        perror("write stm32 cmd failed");
        return -1;
    }
    if(write(fd,"\n",1)!=1)
    {
        perror("write newline failed");
        return -1;
    }
    tcdrain(fd);

    printf("SEND STM32 =%s\n",cmd);
    
    return 0;

}
void stm32_sender_close(int fd)
{
    if(fd>=0)
    {
        close(fd);
    }
}