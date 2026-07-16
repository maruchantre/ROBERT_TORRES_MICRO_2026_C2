#include "rf433.h"

#include "esp_log.h"

static const char *TAG = "rf433";

void rf433_init(void)
{
    ESP_LOGW(TAG, "RF 433 MHz retained but disabled: no documented physical protocol");
}

void rf433_send_packet(const control_packet_t *packet)
{
    (void)packet;
}
