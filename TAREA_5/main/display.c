#include "display.h"

#include "esp_log.h"
#include "app_config.h"

static const char *TAG = "display";
static unsigned page;

void display_init(void)
{
    ESP_LOGW(TAG, "OLED disabled: controller, resolution and I2C address are undocumented");
}

void display_handle_input(const control_input_state_t *input)
{
    if (input->pressed_edges & 0x01) page = (page + 1) % 4;
}

void display_update(const control_packet_t *packet, const calibration_t *calibration,
                    bool wifi_connected, bool mqtt_connected, const char *ip_address)
{
    switch (page) {
    case 0:
        ESP_LOGI(TAG, "MAIN L(%d,%d) R(%d,%d) WiFi=%d MQTT=%d RF=OFF Bat=%lumV Cal=%s %u%%",
                 packet->input.left_x, packet->input.left_y, packet->input.right_x, packet->input.right_y,
                 wifi_connected, mqtt_connected, (unsigned long)packet->input.battery_mv,
                 calibration_status_name(calibration->status), calibration->progress);
        break;
    case 1:
        ESP_LOGI(TAG, "BUTTONS front=0x%X triggers=0x%X press=0x%02X release=0x%02X long=0x%02X",
                 packet->input.front_buttons, packet->input.trigger_buttons, packet->input.pressed_edges,
                 packet->input.released_edges, packet->input.long_press_mask);
        break;
    case 2:
        ESP_LOGI(TAG, "IMU gyro ctrl=(%d,%d,%d) dps=(%.1f,%.1f,%.1f) accel=(%.2f,%.2f,%.2f)",
                 packet->imu.gyro_control[0], packet->imu.gyro_control[1], packet->imu.gyro_control[2],
                 packet->imu.gyro_dps[0], packet->imu.gyro_dps[1], packet->imu.gyro_dps[2],
                 packet->imu.accel_g[0], packet->imu.accel_g[1], packet->imu.accel_g[2]);
        break;
    default:
        ESP_LOGI(TAG, "SYSTEM IP=%s MQTT=%d packets=%lu battery=%lumV calibrated=%d", ip_address,
                 mqtt_connected, (unsigned long)packet->sequence, (unsigned long)packet->input.battery_mv,
                 packet->calibrated);
        break;
    }
}
