CC=gcc
CFLAGS=-Wall -Wextra -g
LDFLAGS=-lmosquitto

TARGET=mqtt_gateway
SRCS=mqtt_gateway.c

all:$(TARGET)

$(TARGET):$(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)