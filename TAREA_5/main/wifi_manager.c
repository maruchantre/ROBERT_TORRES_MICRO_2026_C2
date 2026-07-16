#include "wifi_manager.h"

#include <stdio.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "lwip/inet.h"
#include "pin_config.h"

#if __has_include("secrets.h")
#include "secrets.h"
#define WIFI_SECRETS_CONFIGURED 1
#else
#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define WIFI_SECRETS_CONFIGURED 0
#endif

static const char *TAG = "wifi";
static bool connected;
static char ip_address[16] = "0.0.0.0";

#if !CONTROL_SIMULATION_MODE
static void wifi_event(void *arg, esp_event_base_t base, int32_t event, void *data)
{
    (void)arg;
    if (base == WIFI_EVENT && event == WIFI_EVENT_STA_START) esp_wifi_connect();
    if (base == WIFI_EVENT && event == WIFI_EVENT_STA_DISCONNECTED) {
        connected = false;
        ESP_LOGW(TAG, "Wi-Fi disconnected; retrying");
        esp_wifi_connect();
    }
    if (base == IP_EVENT && event == IP_EVENT_STA_GOT_IP) {
        const ip_event_got_ip_t *got_ip = data;
        inet_ntoa_r(got_ip->ip_info.ip, ip_address, sizeof(ip_address));
        connected = true;
        ESP_LOGI(TAG, "Wi-Fi connected, IP=%s", ip_address);
    }
}
#endif

void wifi_manager_init(void)
{
#if CONTROL_SIMULATION_MODE
    connected = true;
    ESP_LOGI(TAG, "Simulation Wi-Fi connected");
#elif WIFI_SECRETS_CONFIGURED
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t init = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&init));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event, NULL, NULL));
    wifi_config_t config = {0};
    snprintf((char *)config.sta.ssid, sizeof(config.sta.ssid), "%s", WIFI_SSID);
    snprintf((char *)config.sta.password, sizeof(config.sta.password), "%s", WIFI_PASSWORD);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &config));
    ESP_ERROR_CHECK(esp_wifi_start());
#else
    ESP_LOGW(TAG, "Wi-Fi disabled: create main/secrets.h from secrets.example.h");
#endif
}

bool wifi_manager_is_connected(void) { return connected; }
const char *wifi_manager_ip_address(void) { return ip_address; }
