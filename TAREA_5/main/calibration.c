#include "calibration.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "nvs.h"
#include "app_config.h"
#include "pin_config.h"

#define NVS_NAMESPACE "calibration"
#define NVS_KEY "values"
#define CALIBRATION_VERSION 1

typedef struct {
    uint32_t version;
    uint16_t centers[4];
    int32_t gyro_bias[3];
} nvs_calibration_t;

static const char *TAG = "calibration";
static int64_t hold_started_us;
static uint32_t samples;
static uint64_t axis_sum[4];
static int64_t gyro_sum[3];

static void calibration_led_set(int level)
{
#if !CONTROL_SIMULATION_MODE
    gpio_set_level(APP_CALIBRATION_LED, level);
#else
    (void)level;
#endif
}

static bool calibration_triggers_pressed(const control_input_state_t *input)
{
    (void)input;
    return control_input_calibration_triggers_pressed();
}

static void save_calibration(const calibration_t *calibration)
{
    nvs_handle_t nvs = 0;
    const nvs_calibration_t saved = {.version = CALIBRATION_VERSION,
        .centers = {calibration->joystick.center[0], calibration->joystick.center[1],
                    calibration->joystick.center[2], calibration->joystick.center[3]},
        .gyro_bias = {calibration->imu.gyro_bias[0], calibration->imu.gyro_bias[1],
                      calibration->imu.gyro_bias[2]}};
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs) == ESP_OK) {
        ESP_ERROR_CHECK_WITHOUT_ABORT(nvs_set_blob(nvs, NVS_KEY, &saved, sizeof(saved)));
        ESP_ERROR_CHECK_WITHOUT_ABORT(nvs_commit(nvs));
        nvs_close(nvs);
    }
}

void calibration_init(calibration_t *calibration)
{
    *calibration = (calibration_t){.joystick = {.center = {2048, 2048, 2048, 2048}}, .status = CALIBRATION_IDLE};
    nvs_handle_t nvs;
    nvs_calibration_t saved;
    size_t size = sizeof(saved);
    if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs) == ESP_OK) {
        if (nvs_get_blob(nvs, NVS_KEY, &saved, &size) == ESP_OK && size == sizeof(saved) &&
            saved.version == CALIBRATION_VERSION) {
            for (int i = 0; i < 4; ++i) calibration->joystick.center[i] = saved.centers[i];
            for (int i = 0; i < 3; ++i) calibration->imu.gyro_bias[i] = saved.gyro_bias[i];
            calibration->valid = true;
            ESP_LOGI(TAG, "Calibration loaded from NVS");
        }
        nvs_close(nvs);
    }
#if !CONTROL_SIMULATION_MODE
    gpio_config(&(gpio_config_t){.pin_bit_mask = 1ULL << APP_CALIBRATION_LED, .mode = GPIO_MODE_OUTPUT,
                                 .pull_down_en = GPIO_PULLDOWN_DISABLE, .pull_up_en = GPIO_PULLUP_DISABLE});
#endif
}

void calibration_update(calibration_t *calibration, const control_input_state_t *input, const imu_state_t *imu)
{
    const int64_t now = esp_timer_get_time();
    const bool pressed = calibration_triggers_pressed(input);
    if (calibration->status == CALIBRATION_IDLE || calibration->status == CALIBRATION_CANCELLED ||
        calibration->status == CALIBRATION_COMPLETED) {
        if (pressed) {
            calibration->status = CALIBRATION_HOLDING;
            hold_started_us = now;
            calibration->progress = 0;
        }
        return;
    }
    if (!pressed && calibration->status == CALIBRATION_HOLDING) {
        calibration->status = CALIBRATION_CANCELLED;
        calibration->progress = 0;
        calibration_led_set(1);
        ESP_LOGW(TAG, "Calibration cancelled");
        return;
    }
    if (calibration->status == CALIBRATION_HOLDING) {
        const int64_t elapsed = now - hold_started_us;
        calibration->progress = elapsed >= APP_CALIBRATION_HOLD_MS * 1000LL ? 100 :
            (uint8_t)(elapsed / (APP_CALIBRATION_HOLD_MS * 10LL));
        calibration_led_set((elapsed / 250000) % 2);
        if (elapsed >= APP_CALIBRATION_HOLD_MS * 1000LL) {
            calibration->status = CALIBRATION_SAMPLING;
            samples = 0;
            for (int i = 0; i < 4; ++i) axis_sum[i] = 0;
            for (int i = 0; i < 3; ++i) gyro_sum[i] = 0;
        }
        return;
    }
    if (calibration->status == CALIBRATION_SAMPLING) {
        for (int i = 0; i < 4; ++i) axis_sum[i] += input->raw_axis[i];
        for (int i = 0; i < 3; ++i) gyro_sum[i] += imu->raw_gyro[i];
        calibration->progress = (uint8_t)(samples * 100 / APP_CALIBRATION_SAMPLES);
        if (++samples >= APP_CALIBRATION_SAMPLES) {
            for (int i = 0; i < 4; ++i) calibration->joystick.center[i] = axis_sum[i] / samples;
            for (int i = 0; i < 3; ++i) calibration->imu.gyro_bias[i] = gyro_sum[i] / (int32_t)samples;
            calibration->valid = true;
            calibration->progress = 100;
            calibration->status = CALIBRATION_COMPLETED;
            calibration_led_set(1);
            save_calibration(calibration);
            ESP_LOGI(TAG, "CALIBRACION COMPLETADA");
        }
    }
}

const char *calibration_status_name(calibration_status_t status)
{
    static const char *names[] = {"IDLE", "HOLD", "SAMPLING", "COMPLETE", "CANCELLED"};
    return status <= CALIBRATION_CANCELLED ? names[status] : "UNKNOWN";
}
