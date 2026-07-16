#include "control_packet.h"

#include <stddef.h>

control_command_t control_packet_command_from_joystick(uint16_t x, uint16_t y)
{
    const int32_t offset_x = (int32_t)x - CONTROL_JOYSTICK_CENTER;
    const int32_t offset_y = (int32_t)y - CONTROL_JOYSTICK_CENTER;
    const int32_t abs_x = offset_x < 0 ? -offset_x : offset_x;
    const int32_t abs_y = offset_y < 0 ? -offset_y : offset_y;

    if (abs_x <= CONTROL_JOYSTICK_DEAD_ZONE && abs_y <= CONTROL_JOYSTICK_DEAD_ZONE) {
        return CONTROL_CMD_STOP;
    }
    if (abs_y >= abs_x) {
        return offset_y > 0 ? CONTROL_CMD_FORWARD : CONTROL_CMD_BACKWARD;
    }
    return offset_x > 0 ? CONTROL_CMD_RIGHT : CONTROL_CMD_LEFT;
}

void control_packet_finalize(control_packet_t *packet)
{
    uint8_t checksum = 0;
    const uint8_t *bytes = (const uint8_t *)packet;

    packet->header = CONTROL_PACKET_HEADER;
    for (size_t i = 0; i < sizeof(*packet) - sizeof(packet->checksum); ++i) {
        checksum ^= bytes[i];
    }
    packet->checksum = checksum;
}

const char *control_packet_command_name(control_command_t command)
{
    switch (command) {
    case CONTROL_CMD_FORWARD:
        return "FORWARD";
    case CONTROL_CMD_BACKWARD:
        return "BACKWARD";
    case CONTROL_CMD_LEFT:
        return "LEFT";
    case CONTROL_CMD_RIGHT:
        return "RIGHT";
    case CONTROL_CMD_STOP:
    default:
        return "STOP";
    }
}
