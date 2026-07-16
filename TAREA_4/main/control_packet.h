#pragma once

#include <stdint.h>

#define CONTROL_PACKET_HEADER 0xCA43U
#define CONTROL_JOYSTICK_CENTER 2048U
#define CONTROL_JOYSTICK_DEAD_ZONE 350U

typedef enum {
    CONTROL_CMD_STOP = 0,
    CONTROL_CMD_FORWARD,
    CONTROL_CMD_BACKWARD,
    CONTROL_CMD_LEFT,
    CONTROL_CMD_RIGHT,
} control_command_t;

typedef struct __attribute__((packed)) {
    uint16_t header;
    uint16_t sequence;
    uint16_t joystick_left_x;
    uint16_t joystick_left_y;
    uint16_t joystick_right_x;
    uint16_t joystick_right_y;
    uint16_t buttons;
    uint16_t battery_level;
    uint8_t movement_command;
    uint8_t checksum;
} control_packet_t;

control_command_t control_packet_command_from_joystick(uint16_t x, uint16_t y);
void control_packet_finalize(control_packet_t *packet);
const char *control_packet_command_name(control_command_t command);
