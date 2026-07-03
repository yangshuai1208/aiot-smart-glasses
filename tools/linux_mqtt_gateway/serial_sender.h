#ifndef SERIAL_SENDER_H
#define SERIAL_SENDER_H

int serial_sender_open(const char * device,int baudrate);
int serial_sender_send_line(const char * line);
void serial_sender_close(void);

#endif