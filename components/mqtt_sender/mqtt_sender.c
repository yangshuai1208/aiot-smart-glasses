#include "mqtt_sender.h"

#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "esp_event.h"
#include "mqtt_client.h"

#define MQTT_BROKER_URI  "mqtt://192.168.136.1:1883"
#define MQTT_TOPIC_CMD   "aiot/glasses/cmd"

static const char *TAG = "mqtt_sender";

static esp_mqtt_client_handle_t s_mqtt_client = NULL;
static int s_mqtt_connected = 0;
static int s_seq = 0;

static void mqtt_event_handler(void *handler_args,
                               esp_event_base_t base,
                               int32_t event_id,
                               void *event_data)
{
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT connected");
        s_mqtt_connected = 1;
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGW(TAG, "MQTT disconnected");
        s_mqtt_connected = 0;
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT published, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "MQTT event error");
        break;

    default:
        break;
    }
}

void mqtt_sender_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER_URI,
    };

    s_mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (s_mqtt_client == NULL) {
        ESP_LOGE(TAG, "MQTT client init failed");
        return;
    }

    esp_mqtt_client_register_event(s_mqtt_client,
                                   ESP_EVENT_ANY_ID,
                                   mqtt_event_handler,
                                   NULL);

    esp_mqtt_client_start(s_mqtt_client);

    ESP_LOGI(TAG, "MQTT client started, broker=%s", MQTT_BROKER_URI);
}

void mqtt_sender_publish_cmd(const char *cmd)
{
    if (cmd == NULL) {
        return;
    }

    if (s_mqtt_client == NULL || !s_mqtt_connected) {
        ESP_LOGW(TAG, "MQTT not connected, drop cmd=%s", cmd);
        return;
    }

    char payload[128];

    s_seq++;

    snprintf(payload,
             sizeof(payload),
             "{\"device\":\"glasses01\",\"cmd\":\"%s\",\"seq\":%d}",
             cmd,
             s_seq);

    int msg_id = esp_mqtt_client_publish(s_mqtt_client,
                                         MQTT_TOPIC_CMD,
                                         payload,
                                         0,
                                         1,
                                         0);

    ESP_LOGI(TAG,
             "publish topic=%s payload=%s msg_id=%d",
             MQTT_TOPIC_CMD,
             payload,
             msg_id);
}