#include "serial_sender.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

static int g_serial_fd = -1;

static speed_t baudrate_to_speed(int baudrate)
{
    switch (baudrate) {
    case 9600:
        return B9600;

    case 115200:
        return B115200;

    default:
        return B115200;
    }
}

int serial_sender_open(const char *device, int baudrate)
{
    if (device == NULL) {
        return -1;
    }

    g_serial_fd = open(device,
                       O_RDWR | O_NOCTTY | O_SYNC);

    if (g_serial_fd < 0) {
        perror("open serial failed");
        return -1;
    }

    struct termios tty;

    if (tcgetattr(g_serial_fd, &tty) != 0) {
        perror("tcgetattr failed");

        close(g_serial_fd);
        g_serial_fd = -1;

        return -1;
    }

    cfsetospeed(&tty,
                baudrate_to_speed(baudrate));

    cfsetispeed(&tty,
                baudrate_to_speed(baudrate));

    tty.c_cflag =
        (tty.c_cflag & ~CSIZE) | CS8;

    tty.c_cflag |= CLOCAL | CREAD;

    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    tty.c_iflag &=
        ~(IXON | IXOFF | IXANY);

    tty.c_lflag = 0;
    tty.c_oflag = 0;

    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 5;

    if (tcsetattr(g_serial_fd,
                  TCSANOW,
                  &tty) != 0) {
        perror("tcsetattr failed");

        close(g_serial_fd);
        g_serial_fd = -1;

        return -1;
    }

    tcflush(g_serial_fd, TCIOFLUSH);

    printf("Serial opened: %s, baudrate=%d\n",
           device,
           baudrate);

    return 0;
}

int serial_sender_send_line(const char *line)
{
    if (g_serial_fd < 0) {
        printf("serial not opened\n");
        return -1;
    }

    if (line == NULL) {
        return -1;
    }

    char buf[128];

    snprintf(buf,
             sizeof(buf),
             "%s\n",
             line);

    size_t len = strlen(buf);
    size_t total_written = 0;

    while (total_written < len) {
        ssize_t n = write(g_serial_fd,
                          buf + total_written,
                          len - total_written);

        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }

            perror("write serial failed");
            return -1;
        }

        if (n == 0) {
            printf("write serial returned 0\n");
            return -1;
        }

        total_written += (size_t)n;
    }

    if (tcdrain(g_serial_fd) != 0) {
        perror("tcdrain failed");
        return -1;
    }

    printf("UART SEND    = %s", buf);

    return 0;
}

int serial_sender_send_line_retry(const char *line,
                                  int retry_count)
{
    if (retry_count <= 0) {
        retry_count = 1;
    }

    for (int i = 0; i < retry_count; i++) {
        if (serial_sender_send_line(line) == 0) {
            return 0;
        }

        printf("UART retry %d/%d failed, cmd=%s\n",
               i + 1,
               retry_count,
               line);

        usleep(100 * 1000);
    }

    return -1;
}

int serial_sender_read_line(char *buf,
                            size_t buf_size)
{
    if (g_serial_fd < 0) {
        return -1;
    }

    if (buf == NULL || buf_size < 2) {
        return -1;
    }

    size_t index = 0;

    while (1) {
        char ch;

        ssize_t n = read(g_serial_fd,
                         &ch,
                         1);

        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }

            perror("read serial failed");
            return -1;
        }

        /*
         * VTIME 超时，没有收到字节。
         * 继续等待。
         */
        if (n == 0) {
            continue;
        }

        if (ch == '\n') {
            buf[index] = '\0';
            return (int)index;
        }

        /*
         * STM32 ACK 使用 \r\n。
         * 忽略 \r，只使用 \n 作为行结束。
         */
        if (ch == '\r') {
            continue;
        }

        if (index < buf_size - 1) {
            buf[index++] = ch;
        } else {
            buf[0] = '\0';
            return -2;
        }
    }
}

void serial_sender_close(void)
{
    if (g_serial_fd >= 0) {
        close(g_serial_fd);
        g_serial_fd = -1;
    }
}