CC=gcc
CFLAGS=-Wall -Wextra -g
TARGET=uart_receiver
SRCS=uart_receiver.c gateway_dispatcher.c stm32_sender.c

all:
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)