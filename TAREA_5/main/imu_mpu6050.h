#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    int16_t raw_gyro[3];
    int16_t raw_accel[3];
    float gyro_dps[3];
    float accel_g[3];
    float temperature_c;
    int16_t gyro_control[3];
} imu_state_t;

typedef struct {
    int32_t gyro_bias[3];
} imu_calibration_t;

void imu_mpu6050_init(void);
bool imu_mpu6050_read(imu_state_t *state, const imu_calibration_t *calibration);
