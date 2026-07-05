#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <time.h>
#include <ctype.h>
#include <pthread.h>
#include <unistd.h>

#include <mosquitto.h>

#include "serial_sender.h"

#define DEFAULT_BROKER_HOST   "127.0.0.1"
#define DEFAULT_BROKER_PORT   1883

#define DEFAULT_CMD_TOPIC     "aiot/glasses/cmd"
#define STATUS_TOPIC          "aiot/hand/status"

#define DEFAULT_SERIAL_DEVICE "/dev/ttyUSB0"
#define DEFAULT_SERIAL_BAUD   115200

#define CMD_BUF_SIZE          32
#define UART_RETRY_COUNT      3
#define DEDUP_WINDOW_SEC      1

typedef struct
{
    struct mosquitto *mosq;
} serial_rx_context_t;

static void log_with_time(const char *level,
                          const char *fmt,
                          ...)
{
    time_t now = time(NULL);

    struct tm tm_now;

    localtime_r(&now, &tm_now);

    char time_buf[32];

    strftime(time_buf,
             sizeof(time_buf),
             "%H:%M:%S",
             &tm_now);

    printf("[%s][%s] ",
           time_buf,
           level);

    va_list args;

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    printf("\n");
}

static int extract_json_field(const char *json,
                              const char *key,
                              char *out,
                              size_t out_size)
{
    if (json == NULL ||
        key == NULL ||
        out == NULL ||
        out_size == 0) {
        return -1;
    }

    char pattern[64];

    snprintf(pattern,
             sizeof(pattern),
             "\"%s\"",
             key);

    const char *pos = strstr(json, pattern);

    if (pos == NULL) {
        return -1;
    }

    pos += strlen(pattern);

    while (*pos != '\0' &&
           isspace((unsigned char)*pos)) {
        pos++;
    }

    if (*pos != ':') {
        return -1;
    }

    pos++;

    while (*pos != '\0' &&
           isspace((unsigned char)*pos)) {
        pos++;
    }

    if (*pos != '"') {
        return -1;
    }

    pos++;

    const char *end = strchr(pos, '"');

    if (end == NULL) {
        return -1;
    }

    size_t len = (size_t)(end - pos);

    if (len >= out_size) {
        len = out_size - 1;
    }

    memcpy(out, pos, len);

    out[len] = '\0';

    return 0;
}

static const char *cmd_to_stm32_protocol(const char *cmd)
{
    if (cmd == NULL) {
        return "HAND_NONE";
    }

    if (strcmp(cmd, "OPEN") == 0) {
        return "HAND_OPEN";
    }

    if (strcmp(cmd, "GRAB") == 0) {
        return "HAND_GRAB";
    }

    if (strcmp(cmd, "RELEASE") == 0) {
        return "HAND_RELEASE";
    }

    if (strcmp(cmd, "STOP") == 0) {
        return "HAND_STOP";
    }

    return "HAND_NONE";
}

static bool should_drop_duplicate_cmd(
    const char *stm32_cmd)
{
    static char last_cmd[CMD_BUF_SIZE] = {0};
    static time_t last_time = 0;

    if (stm32_cmd == NULL) {
        return true;
    }

    /*
     * STOP 是急停命令。
     * 不参与去重。
     */
    if (strcmp(stm32_cmd,
               "HAND_STOP") == 0) {
        snprintf(last_cmd,
                 sizeof(last_cmd),
                 "%s",
                 stm32_cmd);

        last_time = time(NULL);

        return false;
    }

    time_t now = time(NULL);

    if (strcmp(last_cmd,
               stm32_cmd) == 0 &&
        difftime(now,
                 last_time) < DEDUP_WINDOW_SEC) {
        return true;
    }

    snprintf(last_cmd,
             sizeof(last_cmd),
             "%s",
             stm32_cmd);

    last_time = now;

    return false;
}

static int publish_hand_status(
    struct mosquitto *mosq,
    const char *action,
    const char *status)
{
    if (mosq == NULL ||
        action == NULL ||
        status == NULL) {
        return -1;
    }

    char payload[160];

    snprintf(payload,
             sizeof(payload),
             "{\"device\":\"hand01\","
             "\"action\":\"%s\","
             "\"status\":\"%s\"}",
             action,
             status);

    int msg_id = 0;

    int ret = mosquitto_publish(mosq,
                                &msg_id,
                                STATUS_TOPIC,
                                (int)strlen(payload),
                                payload,
                                1,
                                false);

    if (ret != MOSQ_ERR_SUCCESS) {
        log_with_time(
            "ERROR",
            "status publish failed: %s",
            mosquitto_strerror(ret));

        return -1;
    }

    log_with_time(
        "STATUS",
        "publish topic=%s payload=%s msg_id=%d",
        STATUS_TOPIC,
        payload,
        msg_id);

    return 0;
}

static void handle_serial_ack(
    struct mosquitto *mosq,
    const char *line)
{
    if (line == NULL) {
        return;
    }

    /*
     * 只处理 ACK 消息。
     *
     * PCA9685 init OK 等启动日志直接忽略。
     */
    if (strncmp(line, "ACK:", 4) != 0) {
        log_with_time(
            "UART",
            "ignore non-ACK line=%s",
            line);

        return;
    }

    char action[32] = {0};
    char status[16] = {0};

    int count = sscanf(line,
                       "ACK:%31[^:]:%15s",
                       action,
                       status);

    if (count != 2) {
        log_with_time(
            "WARN",
            "invalid ACK format=%s",
            line);

        return;
    }

    if (strcmp(status, "OK") != 0 &&
        strcmp(status, "FAIL") != 0) {
        log_with_time(
            "WARN",
            "invalid ACK status=%s",
            status);

        return;
    }

    log_with_time(
        "ACK",
        "action=%s status=%s",
        action,
        status);

    publish_hand_status(mosq,
                        action,
                        status);
}

static void *serial_rx_thread(void *arg)
{
    serial_rx_context_t *context =
        (serial_rx_context_t *)arg;

    if (context == NULL ||
        context->mosq == NULL) {
        return NULL;
    }

    char line[128];

    log_with_time(
        "UART",
        "serial RX thread started");

    while (1) {
        int ret = serial_sender_read_line(
            line,
            sizeof(line));

        if (ret == -2) {
            log_with_time(
                "WARN",
                "UART RX line too long, drop");

            continue;
        }

        if (ret < 0) {
            log_with_time(
                "ERROR",
                "UART RX failed");

            usleep(100 * 1000);

            continue;
        }

        if (ret == 0) {
            continue;
        }

        log_with_time(
            "UART",
            "RX = %s",
            line);

        handle_serial_ack(context->mosq,
                          line);
    }

    return NULL;
}

static void handle_payload(const char *payload)
{
    char cmd[CMD_BUF_SIZE];

    log_with_time(
        "MQTT",
        "PAYLOAD = %s",
        payload);

    if (extract_json_field(payload,
                           "cmd",
                           cmd,
                           sizeof(cmd)) != 0) {
        log_with_time(
            "WARN",
            "cmd field not found, drop message");

        return;
    }

    const char *stm32_cmd =
        cmd_to_stm32_protocol(cmd);

    log_with_time(
        "CMD",
        "cmd=%s, stm32_cmd=%s",
        cmd,
        stm32_cmd);

    if (strcmp(stm32_cmd,
               "HAND_NONE") == 0) {
        log_with_time(
            "WARN",
            "invalid cmd=%s, drop message",
            cmd);

        return;
    }

    if (should_drop_duplicate_cmd(stm32_cmd)) {
        log_with_time(
            "DEDUP",
            "drop duplicate cmd=%s",
            stm32_cmd);

        return;
    }

    if (serial_sender_send_line_retry(
            stm32_cmd,
            UART_RETRY_COUNT) != 0) {
        log_with_time(
            "ERROR",
            "UART send failed after retry, cmd=%s",
            stm32_cmd);

        return;
    }

    log_with_time(
        "OK",
        "forward success, cmd=%s",
        stm32_cmd);
}

static void on_connect(struct mosquitto *mosq,
                       void *userdata,
                       int rc)
{
    const char *topic =
        (const char *)userdata;

    if (rc == 0) {
        log_with_time(
            "MQTT",
            "connected");

        int ret = mosquitto_subscribe(
            mosq,
            NULL,
            topic,
            1);

        if (ret != MOSQ_ERR_SUCCESS) {
            log_with_time(
                "ERROR",
                "subscribe failed: %s",
                mosquitto_strerror(ret));
        } else {
            log_with_time(
                "MQTT",
                "subscribed topic: %s",
                topic);
        }
    } else {
        log_with_time(
            "ERROR",
            "connect failed, rc=%d",
            rc);
    }
}

static void on_disconnect(
    struct mosquitto *mosq,
    void *userdata,
    int rc)
{
    (void)mosq;
    (void)userdata;

    log_with_time(
        "WARN",
        "MQTT disconnected, rc=%d",
        rc);
}

static void on_message(
    struct mosquitto *mosq,
    void *userdata,
    const struct mosquitto_message *msg)
{
    (void)mosq;
    (void)userdata;

    if (msg == NULL ||
        msg->payload == NULL) {
        return;
    }

    char *payload = calloc(
        (size_t)msg->payloadlen + 1,
        1);

    if (payload == NULL) {
        log_with_time(
            "ERROR",
            "calloc payload failed");

        return;
    }

    memcpy(payload,
           msg->payload,
           (size_t)msg->payloadlen);

    payload[msg->payloadlen] = '\0';

    log_with_time(
        "MQTT",
        "TOPIC = %s",
        msg->topic);

    handle_payload(payload);

    free(payload);
}

int main(int argc, char *argv[])
{
    const char *host =
        DEFAULT_BROKER_HOST;

    int port =
        DEFAULT_BROKER_PORT;

    const char *cmd_topic =
        DEFAULT_CMD_TOPIC;

    const char *serial_device =
        DEFAULT_SERIAL_DEVICE;

    int serial_baud =
        DEFAULT_SERIAL_BAUD;

    if (argc >= 2) {
        host = argv[1];
    }

    if (argc >= 3) {
        port = atoi(argv[2]);
    }

    if (argc >= 4) {
        cmd_topic = argv[3];
    }

    if (argc >= 5) {
        serial_device = argv[4];
    }

    if (argc >= 6) {
        serial_baud = atoi(argv[5]);
    }

    log_with_time(
        "INFO",
        "Linux MQTT Gateway started");

    log_with_time(
        "INFO",
        "Broker host  : %s",
        host);

    log_with_time(
        "INFO",
        "Broker port  : %d",
        port);

    log_with_time(
        "INFO",
        "CMD topic    : %s",
        cmd_topic);

    log_with_time(
        "INFO",
        "STATUS topic : %s",
        STATUS_TOPIC);

    log_with_time(
        "INFO",
        "Serial device: %s",
        serial_device);

    log_with_time(
        "INFO",
        "Serial baud  : %d",
        serial_baud);

    if (serial_sender_open(
            serial_device,
            serial_baud) != 0) {
        log_with_time(
            "ERROR",
            "serial open failed, exit");

        return 1;
    }

    mosquitto_lib_init();

    struct mosquitto *mosq =
        mosquitto_new(
            "linux_gateway_01",
            true,
            (void *)cmd_topic);

    if (mosq == NULL) {
        log_with_time(
            "ERROR",
            "mosquitto_new failed");

        serial_sender_close();

        mosquitto_lib_cleanup();

        return 1;
    }

    /*
     * MQTT API 会在主线程和串口接收线程中使用。
     */
    int ret = mosquitto_threaded_set(
        mosq,
        true);

    if (ret != MOSQ_ERR_SUCCESS) {
        log_with_time(
            "ERROR",
            "mosquitto_threaded_set failed: %s",
            mosquitto_strerror(ret));

        mosquitto_destroy(mosq);

        serial_sender_close();

        mosquitto_lib_cleanup();

        return 1;
    }

    mosquitto_reconnect_delay_set(
        mosq,
        2,
        10,
        true);

    mosquitto_connect_callback_set(
        mosq,
        on_connect);

    mosquitto_disconnect_callback_set(
        mosq,
        on_disconnect);

    mosquitto_message_callback_set(
        mosq,
        on_message);

    ret = mosquitto_connect(
        mosq,
        host,
        port,
        60);

    if (ret != MOSQ_ERR_SUCCESS) {
        log_with_time(
            "ERROR",
            "mosquitto_connect failed: %s",
            mosquitto_strerror(ret));

        mosquitto_destroy(mosq);

        serial_sender_close();

        mosquitto_lib_cleanup();

        return 1;
    }

    serial_rx_context_t rx_context = {
        .mosq = mosq
    };

    pthread_t rx_thread;

    ret = pthread_create(
        &rx_thread,
        NULL,
        serial_rx_thread,
        &rx_context);

    if (ret != 0) {
        log_with_time(
            "ERROR",
            "pthread_create failed, ret=%d",
            ret);

        mosquitto_destroy(mosq);

        serial_sender_close();

        mosquitto_lib_cleanup();

        return 1;
    }

    ret = mosquitto_loop_forever(
        mosq,
        -1,
        1);

    if (ret != MOSQ_ERR_SUCCESS) {
        log_with_time(
            "ERROR",
            "mosquitto_loop_forever failed: %s",
            mosquitto_strerror(ret));
    }

    pthread_cancel(rx_thread);
    pthread_join(rx_thread, NULL);

    mosquitto_destroy(mosq);

    serial_sender_close();

    mosquitto_lib_cleanup();

    return 0;
}