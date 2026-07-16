#include "imu_mpu6050.h"

#include "driver/i2c_master.h"
#include "esp_check.h"
#include "esp_log.h"
#include "app_config.h"
#include "pin_config.h"

#define MPU6050_ADDRESS 0x68
#define MPU6050_REG_WHO_AM_I 0x75
#define MPU6050_REG_PWR_MGMT_1 0x6B
#define MPU6050_REG_CONFIG 0x1A
#define MPU6050_REG_GYRO_CONFIG 0x1B
#define MPU6050_REG_ACCEL_CONFIG 0x1C
#define MPU6050_REG_ACCEL_XOUT_H 0x3B

static const char *TAG = "mpu6050";
#if !CONTROL_SIMULATION_MODE
static i2c_master_dev_handle_t mpu;

static int16_t be16(const uint8_t *bytes)
{
    return (int16_t)((bytes[0] << 8) | bytes[1]);
}
#endif

void imu_mpu6050_init(void)
{
#if CONTROL_SIMULATION_MODE
    ESP_LOGI(TAG, "Simulation enabled: MPU-6050 is synthetic");
#else
    i2c_master_bus_handle_t bus;
    ESP_ERROR_CHECK(i2c_new_master_bus(&(i2c_master_bus_config_t){
        .i2c_port = I2C_NUM_0, .sda_io_num = PIN_I2C_SDA, .scl_io_num = PIN_I2C_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT, .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = false,
    }, &bus));
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus, &(i2c_device_config_t){
        .dev_addr_length = I2C_ADDR_BIT_LEN_7, .device_address = MPU6050_ADDRESS,
        .scl_speed_hz = 400000,
    }, &mpu));
    uint8_t who_am_i;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(mpu, (uint8_t[]){MPU6050_REG_WHO_AM_I}, 1,
                                                &who_am_i, 1, 100));
    ESP_ERROR_CHECK_WITHOUT_ABORT(who_am_i == MPU6050_ADDRESS ? ESP_OK : ESP_FAIL);
    ESP_ERROR_CHECK(i2c_master_transmit(mpu, (uint8_t[]){MPU6050_REG_PWR_MGMT_1, 0x01}, 2, 100));
    ESP_ERROR_CHECK(i2c_master_transmit(mpu, (uint8_t[]){MPU6050_REG_CONFIG, 0x03}, 2, 100));
    ESP_ERROR_CHECK(i2c_master_transmit(mpu, (uint8_t[]){MPU6050_REG_GYRO_CONFIG, 0x00}, 2, 100));
    ESP_ERROR_CHECK(i2c_master_transmit(mpu, (uint8_t[]){MPU6050_REG_ACCEL_CONFIG, 0x00}, 2, 100));
    ESP_LOGI(TAG, "MPU-6050 found at 0x%02X", who_am_i);
#endif
}

bool imu_mpu6050_read(imu_state_t *state, const imu_calibration_t *calibration)
{
    static uint32_t phase;
    *state = (imu_state_t){0};
#if CONTROL_SIMULATION_MODE
    phase++;
    state->raw_gyro[0] = (int16_t)((phase % 200) - 100) * 100;
    state->raw_gyro[1] = (int16_t)((phase % 150) - 75) * 80;
    state->raw_gyro[2] = (int16_t)((phase % 100) - 50) * 60;
    state->raw_accel[0] = (int16_t)((phase % 20) - 10) * 100;
    state->raw_accel[1] = (int16_t)((phase % 16) - 8) * 100;
    state->raw_accel[2] = 16384;
    state->temperature_c = 25.0f;
#else
    uint8_t data[14];
    if (i2c_master_transmit_receive(mpu, (uint8_t[]){MPU6050_REG_ACCEL_XOUT_H}, 1, data,
                                    sizeof(data), 100) != ESP_OK) {
        ESP_LOGW(TAG, "MPU-6050 read failed");
        return false;
    }
    for (int i = 0; i < 3; ++i) state->raw_accel[i] = be16(&data[i * 2]);
    state->temperature_c = (float)be16(&data[6]) / 340.0f + 36.53f;
    for (int i = 0; i < 3; ++i) state->raw_gyro[i] = be16(&data[8 + i * 2]);
#endif
    for (int i = 0; i < 3; ++i) {
        const int32_t corrected = (int32_t)state->raw_gyro[i] - calibration->gyro_bias[i];
        state->gyro_dps[i] = (float)corrected / 131.0f;
        state->accel_g[i] = (float)state->raw_accel[i] / 16384.0f;
        int value = (int)(state->gyro_dps[i] * 100.0f / APP_GYRO_FULL_SCALE_DPS);
        if (value > 100) value = 100;
        if (value < -100) value = -100;
        state->gyro_control[i] = value >= -APP_GYRO_DEAD_ZONE && value <= APP_GYRO_DEAD_ZONE ? 0 : value;
    }
    return true;
}
