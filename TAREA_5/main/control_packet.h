#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "calibration.h"
#include "control_input.h"
#include "imu_mpu6050.h"

typedef struct {
    uint32_t sequence;
    control_input_state_t input;
    imu_state_t imu;
    bool calibrated;
} control_packet_t;

void control_packet_build(control_packet_t *packet, uint32_t sequence, const control_input_state_t *input,
                          const imu_state_t *imu, const calibration_t *calibration);
int control_packet_to_json(const control_packet_t *packet, char *buffer, size_t buffer_size);
