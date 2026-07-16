#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    CONTROL_BUTTON_FRONT_0 = 0,
    CONTROL_BUTTON_FRONT_1,
    CONTROL_BUTTON_FRONT_2,
    CONTROL_BUTTON_FRONT_3,
    CONTROL_BUTTON_TRIGGER_0,
    CONTROL_BUTTON_TRIGGER_1,
    CONTROL_BUTTON_TRIGGER_2,
    CONTROL_BUTTON_TRIGGER_3,
    CONTROL_BUTTON_COUNT,
} control_button_id_t;

typedef struct {
    int16_t left_x;
    int16_t left_y;
    int16_t right_x;
    int16_t right_y;
    uint16_t raw_axis[4];
    uint8_t front_buttons;
    uint8_t trigger_buttons;
    uint8_t pressed_edges;
    uint8_t released_edges;
    uint8_t long_press_mask;
    uint16_t battery_raw;
    uint32_t battery_mv;
} control_input_state_t;

typedef struct {
    uint16_t center[4];
} joystick_calibration_t;

void control_input_init(void);
void control_input_read(control_input_state_t *state, const joystick_calibration_t *calibration);
bool control_input_button_pressed(control_button_id_t button);
bool control_input_calibration_triggers_pressed(void);
