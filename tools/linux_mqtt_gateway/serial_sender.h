#ifndef SERIAL_SENDER_H
#define SERIAL_SENDER_H

#include <stddef.h>

int serial_sender_open(const char *device, int baudrate);

int serial_sender_send_line(const char *line);

int serial_sender_send_line_retry(const char *line,
                                  int retry_count);

int serial_sender_read_line(char *buf,
                            size_t buf_size);

void serial_sender_close(void);

#endif