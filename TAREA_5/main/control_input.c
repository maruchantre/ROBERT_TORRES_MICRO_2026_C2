#include "control_input.h"

#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "app_config.h"
#include "pin_config.h"

#define BUTTON_DEBOUNCE_US 40000
#define BUTTON_LONG_PRESS_US 800000

static const char *TAG = "control_input";
static const gpio_num_t button_pins[CONTROL_BUTTON_COUNT] = {
    PIN_BTN_0, PIN_BTN_1, PIN_BTN_2, PIN_BTN_3,
    PIN_BTNL4, PIN_BTNL3, PIN_BTNL2, PIN_BTNL1,
};
#if !CONTROL_SIMULATION_MODE
static const adc_channel_t adc_channels[] = {
    ADC_CHANNEL_4, ADC_CHANNEL_3, ADC_CHANNEL_0, ADC_CHANNEL_1, ADC_CHANNEL_7,
};
static adc_oneshot_unit_handle_t adc_handle;
static uint16_t filtered_raw[5];
#endif
static int sampled_level[CONTROL_BUTTON_COUNT];
static int stable_level[CONTROL_BUTTON_COUNT];
static int64_t level_changed_us[CONTROL_BUTTON_COUNT];
static int64_t pressed_since_us[CONTROL_BUTTON_COUNT];

static int16_t normalize_axis(uint16_t raw, uint16_t center)
{
    int value;
    if (raw >= center) {
        const uint16_t span = center < 4095 ? 4095 - center : 1;
        value = ((int32_t)(raw - center) * 100 + span / 2) / span;
    } else {
        const uint16_t span = center > 0 ? center : 1;
        value = -((int32_t)(center - raw) * 100 + span / 2) / span;
    }
    if (value > 100) value = 100;
    if (value < -100) value = -100;
    return value >= -APP_JOYSTICK_DEAD_ZONE && value <= APP_JOYSTICK_DEAD_ZONE ? 0 : value;
}

void control_input_init(void)
{
    const int64_t now = esp_timer_get_time();
#if CONTROL_SIMULATION_MODE
    ESP_LOGI(TAG, "Simulation enabled: ADC and GPIO are synthetic");
#else
    uint64_t mask = 0;
    for (int i = 0; i < CONTROL_BUTTON_COUNT; ++i) mask |= 1ULL << button_pins[i];
    ESP_ERROR_CHECK(gpio_config(&(gpio_config_t){
        .pin_bit_mask = mask, .mode = GPIO_MODE_INPUT, .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE, .intr_type = GPIO_INTR_DISABLE,
    }));
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&(adc_oneshot_unit_init_cfg_t){.unit_id = ADC_UNIT_1}, &adc_handle));
    for (size_t i = 0; i < sizeof(adc_channels) / sizeof(adc_channels[0]); ++i) {
        ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, adc_channels[i],
            &(adc_oneshot_chan_cfg_t){.atten = ADC_ATTEN_DB_12, .bitwidth = ADC_BITWIDTH_DEFAULT}));
    }
#endif
    for (int i = 0; i < CONTROL_BUTTON_COUNT; ++i) {
#if CONTROL_SIMULATION_MODE
        stable_level[i] = sampled_level[i] = 1;
#else
        stable_level[i] = sampled_level[i] = gpio_get_level(button_pins[i]);
#endif
        level_changed_us[i] = now;
    }
}

bool control_input_button_pressed(control_button_id_t button)
{
    return button < CONTROL_BUTTON_COUNT && stable_level[button] == 0;
}

bool control_input_calibration_triggers_pressed(void)
{
    bool first = false;
    bool second = false;
    for (int i = 0; i < CONTROL_BUTTON_COUNT; ++i) {
        if (button_pins[i] == PIN_CALIBRATION_TRIGGER_A) first = stable_level[i] == 0;
        if (button_pins[i] == PIN_CALIBRATION_TRIGGER_B) second = stable_level[i] == 0;
    }
    return first && second;
}

#if !CONTROL_SIMULATION_MODE
static uint16_t read_adc_average(size_t channel)
{
    int value;
    int32_t sum = 0;
    for (int sample = 0; sample < APP_ADC_SAMPLE_COUNT; ++sample) {
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, adc_channels[channel], &value));
        sum += value;
    }
    return (uint16_t)(sum / APP_ADC_SAMPLE_COUNT);
}
#endif

void control_input_read(control_input_state_t *state, const joystick_calibration_t *calibration)
{
    static uint32_t sequence;
    const int64_t now = esp_timer_get_time();
    *state = (control_input_state_t){0};
#if CONTROL_SIMULATION_MODE
    const int phase = sequence++ % 200;
    const int triangle = phase < 100 ? phase : 200 - phase;
    const int values[] = {triangle * 2 - 100, 100 - triangle * 2, (phase % 120) - 60, 60 - (phase % 120)};
    for (int i = 0; i < 4; ++i) {
        state->raw_axis[i] = (uint16_t)(2048 + values[i] * 2047 / 100);
    }
    state->battery_raw = 3000;
#else
    for (int i = 0; i < 5; ++i) {
        const uint16_t raw = read_adc_average(i);
        filtered_raw[i] = filtered_raw[i] == 0 ? raw :
            (uint16_t)(filtered_raw[i] + ((int)raw - filtered_raw[i]) / (1 << APP_ADC_FILTER_SHIFT));
    }
    for (int i = 0; i < 4; ++i) state->raw_axis[i] = filtered_raw[i];
    state->battery_raw = filtered_raw[4];
#endif
    uint8_t current_buttons = 0;
    for (int i = 0; i < CONTROL_BUTTON_COUNT; ++i) {
#if CONTROL_SIMULATION_MODE
        /* Both configurable calibration candidates are held periodically. */
        const int level = (button_pins[i] == PIN_CALIBRATION_TRIGGER_A ||
                           button_pins[i] == PIN_CALIBRATION_TRIGGER_B) && sequence % 400U < 180U ? 0 :
                          ((sequence / 25 + i) % 17 == 0 ? 0 : 1);
#else
        const int level = gpio_get_level(button_pins[i]);
#endif
        if (level != sampled_level[i]) {
            sampled_level[i] = level;
            level_changed_us[i] = now;
        }
        if (stable_level[i] != sampled_level[i] && now - level_changed_us[i] >= BUTTON_DEBOUNCE_US) {
            stable_level[i] = sampled_level[i];
            if (stable_level[i] == 0) {
                state->pressed_edges |= 1U << i;
                pressed_since_us[i] = now;
            } else {
                state->released_edges |= 1U << i;
                pressed_since_us[i] = 0;
            }
        }
        if (stable_level[i] == 0) {
            current_buttons |= 1U << i;
            if (now - pressed_since_us[i] >= BUTTON_LONG_PRESS_US) state->long_press_mask |= 1U << i;
        }
    }
    state->front_buttons = current_buttons & 0x0f;
    state->trigger_buttons = (current_buttons >> 4) & 0x0f;
    state->left_x = normalize_axis(state->raw_axis[0], calibration->center[0]);
    state->left_y = normalize_axis(state->raw_axis[1], calibration->center[1]);
    state->right_x = normalize_axis(state->raw_axis[2], calibration->center[2]);
    state->right_y = normalize_axis(state->raw_axis[3], calibration->center[3]);
    state->battery_mv = (uint32_t)state->battery_raw * 3300 / 4095;
}
