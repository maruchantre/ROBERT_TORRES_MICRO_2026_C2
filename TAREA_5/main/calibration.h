#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "control_input.h"
#include "imu_mpu6050.h"

typedef enum {
    CALIBRATION_IDLE,
    CALIBRATION_HOLDING,
    CALIBRATION_SAMPLING,
    CALIBRATION_COMPLETED,
    CALIBRATION_CANCELLED,
} calibration_status_t;

typedef struct {
    joystick_calibration_t joystick;
    imu_calibration_t imu;
    bool valid;
    calibration_status_t status;
    uint8_t progress;
} calibration_t;

void calibration_init(calibration_t *calibration);
void calibration_update(calibration_t *calibration, const control_input_state_t *input,
                        const imu_state_t *imu);
const char *calibration_status_name(calibration_status_t status);
