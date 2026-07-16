#include "rf433.h"

#include "esp_log.h"
#include "pin_config.h"

static const char *TAG = "rf433";

void rf433_init(void)
{
#if CONTROL_SIMULATION_MODE
    ESP_LOGI(TAG, "Simulation: RF transmitter disabled");
#else
    ESP_LOGW(TAG, "RF 433 MHz protocol is undocumented; physical transmission remains disabled");
#endif
}

void rf433_send_packet(const control_packet_t *packet)
{
#if CONTROL_SIMULATION_MODE
    ESP_LOGI(TAG,
             "SIM packet hdr=0x%04X seq=%u L=(%u,%u) R=(%u,%u) buttons=0x%04X bat=%u cmd=%s chk=0x%02X",
             packet->header, packet->sequence, packet->joystick_left_x, packet->joystick_left_y,
             packet->joystick_right_x, packet->joystick_right_y, packet->buttons,
             packet->battery_level, control_packet_command_name((control_command_t)packet->movement_command),
             packet->checksum);
#else
    (void)packet;
    ESP_LOGW(TAG, "Packet not transmitted: define the documented RF 433 MHz protocol first");
#endif
}
