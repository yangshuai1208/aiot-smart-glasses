#ifndef MQTT_SENDER_H
#define MQTT_SENDER_H

void mqtt_sender_start(void);
void mqtt_sender_publish_cmd(const char *cmd);

#endif