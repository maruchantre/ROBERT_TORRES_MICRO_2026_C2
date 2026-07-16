#include "mqtt_manager.h"

#include <stdio.h>

#include "esp_log.h"
#include "esp_mac.h"
#include "mqtt_client.h"
#include "app_config.h"

static const char *TAG = "mqtt";
#if !CONTROL_SIMULATION_MODE
static esp_mqtt_client_handle_t client;
static char topic_prefix[96] = APP_MQTT_TOPIC_PREFIX;
static char client_id[32];
#endif
static bool connected;

#if !CONTROL_SIMULATION_MODE
static void mqtt_event(void *arg, esp_event_base_t base, int32_t event_id, void *event_data)
{
    (void)arg;
    (void)base;
    (void)event_data;
    if (event_id == MQTT_EVENT_CONNECTED) {
        connected = true;
        char topic[112];
        snprintf(topic, sizeof(topic), "%s/status", topic_prefix);
        esp_mqtt_client_publish(client, topic, "online", 0, 1, 1);
        ESP_LOGI(TAG, "MQTT connected");
    } else if (event_id == MQTT_EVENT_DISCONNECTED) {
        connected = false;
        ESP_LOGW(TAG, "MQTT disconnected");
    } else if (event_id == MQTT_EVENT_ERROR) {
        ESP_LOGE(TAG, "MQTT error");
    }
}
#endif

void mqtt_manager_init(void)
{
    ESP_LOGI(TAG, "MQTT ready; broker=%s topic=%s/state", APP_MQTT_BROKER_URI,
             APP_MQTT_TOPIC_PREFIX);
}

void mqtt_manager_start(void)
{
#if CONTROL_SIMULATION_MODE
    connected = true;
    ESP_LOGI(TAG, "Simulation MQTT connected; JSON logged only");
#else
    if (client != NULL) return;
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    snprintf(client_id, sizeof(client_id), "%s-%02X%02X%02X", APP_DEVICE_NAME, mac[3], mac[4], mac[5]);
    const esp_mqtt_client_config_t config = {
        .broker.address.uri = APP_MQTT_BROKER_URI,
        .credentials.client_id = client_id,
        .session.last_will.topic = APP_MQTT_TOPIC_PREFIX "/status",
        .session.last_will.msg = "offline",
        .session.last_will.qos = 1,
        .session.last_will.retain = true,
    };
    client = esp_mqtt_client_init(&config);
    ESP_ERROR_CHECK(esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event, NULL));
    ESP_ERROR_CHECK(esp_mqtt_client_start(client));
#endif
}

#if !CONTROL_SIMULATION_MODE
static void publish_value(const char *suffix, const char *value)
{
    if (!connected || !client) return;
    char topic[128];
    snprintf(topic, sizeof(topic), "%s/%s", topic_prefix, suffix);
    esp_mqtt_client_publish(client, topic, value, 0, 0, 0);
}

static void publish_int(const char *suffix, int value)
{
    char text[16];
    snprintf(text, sizeof(text), "%d", value);
    publish_value(suffix, text);
}
#endif

void mqtt_manager_publish(const control_packet_t *packet, bool buttons_changed)
{
    char json[512];
    control_packet_to_json(packet, json, sizeof(json));
#if CONTROL_SIMULATION_MODE
    ESP_LOGI(TAG, "SIM MQTT " APP_MQTT_TOPIC_PREFIX "/state %s", json);
    (void)buttons_changed;
#else
    if (!wifi_manager_is_connected() || !connected) return;
    publish_value("state", json);
    publish_int("joystick/left/x", packet->input.left_x);
    publish_int("joystick/left/y", packet->input.left_y);
    publish_int("joystick/right/x", packet->input.right_x);
    publish_int("joystick/right/y", packet->input.right_y);
    publish_int("gyro/x", packet->imu.gyro_control[0]);
    publish_int("gyro/y", packet->imu.gyro_control[1]);
    publish_int("gyro/z", packet->imu.gyro_control[2]);
    char value[16];
    for (int i = 0; i < 3; ++i) {
        snprintf(value, sizeof(value), "%.3f", packet->imu.accel_g[i]);
        publish_value(i == 0 ? "accel/x" : i == 1 ? "accel/y" : "accel/z", value);
    }
    publish_int("buttons/front", packet->input.front_buttons);
    publish_int("buttons/triggers", packet->input.trigger_buttons);
    publish_int("battery", packet->input.battery_mv);
    ESP_LOGI(TAG, "Published %s/state %s", topic_prefix, json);
    (void)buttons_changed;
#endif
}

bool mqtt_manager_is_connected(void) { return connected; }
