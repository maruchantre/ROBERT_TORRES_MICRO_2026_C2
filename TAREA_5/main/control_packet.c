#include "control_packet.h"

#include <stdio.h>

void control_packet_build(control_packet_t *packet, uint32_t sequence, const control_input_state_t *input,
                          const imu_state_t *imu, const calibration_t *calibration)
{
    *packet = (control_packet_t){.sequence = sequence, .input = *input, .imu = *imu,
                                 .calibrated = calibration->valid};
}

int control_packet_to_json(const control_packet_t *packet, char *buffer, size_t buffer_size)
{
    return snprintf(buffer, buffer_size,
        "{\"sequence\":%lu,\"joystick\":{\"left_x\":%d,\"left_y\":%d,\"right_x\":%d,\"right_y\":%d},"
        "\"gyro\":{\"x\":%d,\"y\":%d,\"z\":%d},\"gyro_dps\":{\"x\":%.2f,\"y\":%.2f,\"z\":%.2f},"
        "\"accel_g\":{\"x\":%.3f,\"y\":%.3f,\"z\":%.3f},\"temperature_c\":%.2f,"
        "\"front_buttons\":%u,\"trigger_buttons\":%u,\"battery_mv\":%lu,\"calibrated\":%s}",
        (unsigned long)packet->sequence, packet->input.left_x, packet->input.left_y,
        packet->input.right_x, packet->input.right_y, packet->imu.gyro_control[0],
        packet->imu.gyro_control[1], packet->imu.gyro_control[2], packet->imu.gyro_dps[0],
        packet->imu.gyro_dps[1], packet->imu.gyro_dps[2], packet->imu.accel_g[0],
        packet->imu.accel_g[1], packet->imu.accel_g[2], packet->imu.temperature_c,
        packet->input.front_buttons, packet->input.trigger_buttons,
        (unsigned long)packet->input.battery_mv, packet->calibrated ? "true" : "false");
}
