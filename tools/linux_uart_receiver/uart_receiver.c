#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

#define LINE_BUF_SIZE   256
#define CMD_BUF_SIZE    64

static int uart_open_and_config(const char *device)
{
    int fd = open(device, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        perror("open uart failed");
        return -1;
    }

    struct termios tty;
    memset(&tty, 0, sizeof(tty));

    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr failed");
        close(fd);
        return -1;
    }

    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    tty.c_cflag &= ~PARENB;          // 无校验
    tty.c_cflag &= ~CSTOPB;          // 1 位停止位
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;              // 8 位数据位
    tty.c_cflag &= ~CRTSCTS;         // 关闭硬件流控
    tty.c_cflag |= CREAD | CLOCAL;   // 开启接收，忽略调制解调器控制线

    tty.c_lflag &= ~ICANON;          // 非规范模式
    tty.c_lflag &= ~ECHO;
    tty.c_lflag &= ~ECHOE;
    tty.c_lflag &= ~ISIG;

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);  // 关闭软件流控
    tty.c_iflag &= ~(ICRNL | INLCR);         // 不转换换行符

    tty.c_oflag &= ~OPOST;            // 原始输出模式

    tty.c_cc[VMIN] = 1;               // 至少读取 1 字节
    tty.c_cc[VTIME] = 0;              // 不设置超时

    tcflush(fd, TCIFLUSH);

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr failed");
        close(fd);
        return -1;
    }

    return fd;
}

static int extract_json_field(const char *json,
                              const char *key,
                              char *out,
                              size_t out_size)
{
    if (json == NULL || key == NULL || out == NULL || out_size == 0) {
        return -1;
    }

    char pattern[64];

    snprintf(pattern, sizeof(pattern), "\"%s\":\"", key);

    char *start = strstr(json, pattern);
    if (start == NULL) {
        return -1;
    }

    start += strlen(pattern);

    char *end = strchr(start, '"');
    if (end == NULL) {
        return -1;
    }

    size_t len = (size_t)(end - start);
    if (len >= out_size) {
        len = out_size - 1;
    }

    memcpy(out, start, len);
    out[len] = '\0';

    return 0;
}

static void handle_json_line(const char *line)
{
    char cmd[CMD_BUF_SIZE];
    char mode[CMD_BUF_SIZE];
    char gesture[CMD_BUF_SIZE];
    char status[CMD_BUF_SIZE];

    printf("RX JSON: %s\n", line);

    if (extract_json_field(line, "mode", mode, sizeof(mode)) == 0) {
        printf("  mode    = %s\n", mode);
    }

    if (extract_json_field(line, "gesture", gesture, sizeof(gesture)) == 0) {
        printf("  gesture = %s\n", gesture);
    }

    if (extract_json_field(line, "cmd", cmd, sizeof(cmd)) == 0) {
        printf("  cmd     = %s\n", cmd);
    }

    if (extract_json_field(line, "status", status, sizeof(status)) == 0) {
        printf("  status  = %s\n", status);
    }

    if (extract_json_field(line, "cmd", cmd, sizeof(cmd)) == 0) {
        if (strcmp(cmd, "OPEN") == 0) {
            printf("  ACTION  = open hand\n");
        } else if (strcmp(cmd, "GRAB") == 0) {
            printf("  ACTION  = grab object\n");
        } else if (strcmp(cmd, "RELEASE") == 0) {
            printf("  ACTION  = release object\n");
        } else if (strcmp(cmd, "STOP") == 0) {
            printf("  ACTION  = emergency stop\n");
        } else {
            printf("  ACTION  = none\n");
        }
    }

    printf("--------------------------------\n");
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <uart_device>\n", argv[0]);
        printf("Example: %s /dev/ttyUSB0\n", argv[0]);
        return 1;
    }

    const char *device = argv[1];

    int fd = uart_open_and_config(device);
    if (fd < 0) {
        return 1;
    }

    printf("Linux UART JSON Receiver started\n");
    printf("Device: %s\n", device);
    printf("Baudrate: 115200\n");
    printf("--------------------------------\n");

    char line_buf[LINE_BUF_SIZE];
    int index = 0;

    while (1) {
        char ch;
        int n = read(fd, &ch, 1);

        if (n > 0) {
            if (ch == '\n') {
                line_buf[index] = '\0';

                if (index > 0) {
                    handle_json_line(line_buf);
                }

                index = 0;
            } else if (ch != '\r') {
                if (index < LINE_BUF_SIZE - 1) {
                    line_buf[index++] = ch;
                } else {
                    index = 0;
                    printf("line too long, drop\n");
                }
            }
        } else if (n < 0) {
            perror("read failed");
            break;
        }
    }

    close(fd);
    return 0;
}