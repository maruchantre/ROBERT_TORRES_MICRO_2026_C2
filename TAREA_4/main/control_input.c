#include "control_input.h"

#include <stddef.h>

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_check.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pin_config.h"

#define BUTTON_DEBOUNCE_MS 40

static const char *TAG = "control_input";
static uint16_t sequence_number;

#if !CONTROL_SIMULATION_MODE
typedef struct {
    gpio_num_t pin;
    int stable_level;
    int sampled_level;
    TickType_t last_change;
} button_t;

static button_t buttons[] = {
    {.pin = PIN_JOY0_BTN}, {.pin = PIN_BTN_0}, {.pin = PIN_BTN_2},
    {.pin = PIN_BTN_1}, {.pin = PIN_BTN_3}, {.pin = PIN_BTNL4},
    {.pin = PIN_BTNL3}, {.pin = PIN_BTNL2}, {.pin = PIN_BTNL1},
    {.pin = PIN_JOY1_BTN},
};

static const adc_channel_t adc_channels[] = {
    ADC_CHANNEL_4, /* JOY0_MD: left X, GPIO5 */
    ADC_CHANNEL_3, /* JOY0_MT: left Y, GPIO4 */
    ADC_CHANNEL_0, /* JOY1_MD: right X, GPIO1 */
    ADC_CHANNEL_1, /* JOY1_MT: right Y, GPIO2 */
    ADC_CHANNEL_7, /* VBAT, GPIO8 */
};
static adc_oneshot_unit_handle_t adc_handle;

static uint16_t read_buttons(void)
{
    const TickType_t now = xTaskGetTickCount();
    uint16_t pressed = 0;

    for (size_t i = 0; i < sizeof(buttons) / sizeof(buttons[0]); ++i) {
        button_t *button = &buttons[i];
        const int level = gpio_get_level(button->pin);
        if (level != button->sampled_level) {
            button->sampled_level = level;
            button->last_change = now;
        }
        if (button->stable_level != button->sampled_level &&
            now - button->last_change >= pdMS_TO_TICKS(BUTTON_DEBOUNCE_MS)) {
            button->stable_level = button->sampled_level;
        }
        if (button->stable_level == 0) {
            pressed |= (uint16_t)(1U << i);
        }
    }
    return pressed;
}
#endif

void control_input_init(void)
{
#if CONTROL_SIMULATION_MODE
    ESP_LOGI(TAG, "Simulation enabled: GPIO, ADC and I2C are not initialized");
#else
    uint64_t button_mask = 0;
    for (size_t i = 0; i < sizeof(buttons) / sizeof(buttons[0]); ++i) {
        button_mask |= 1ULL << buttons[i].pin;
    }
    const gpio_config_t button_config = {
        .pin_bit_mask = button_mask,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&button_config));
    const TickType_t now = xTaskGetTickCount();
    for (size_t i = 0; i < sizeof(buttons) / sizeof(buttons[0]); ++i) {
        buttons[i].stable_level = gpio_get_level(buttons[i].pin);
        buttons[i].sampled_level = buttons[i].stable_level;
        buttons[i].last_change = now;
    }

    const adc_oneshot_unit_init_cfg_t unit_config = {.unit_id = ADC_UNIT_1};
    const adc_oneshot_chan_cfg_t channel_config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_config, &adc_handle));
    for (size_t i = 0; i < sizeof(adc_channels) / sizeof(adc_channels[0]); ++i) {
        ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, adc_channels[i], &channel_config));
    }

    const i2c_master_bus_config_t i2c_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = PIN_I2C_SDA,
        .scl_io_num = PIN_I2C_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = false,
    };
    i2c_master_bus_handle_t i2c_bus;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_config, &i2c_bus));
    ESP_LOGI(TAG, "Real inputs initialized; I2C SDA=%d SCL=%d", PIN_I2C_SDA, PIN_I2C_SCL);
#endif
}

void control_input_read(control_packet_t *packet)
{
    packet->sequence = sequence_number++;
#if CONTROL_SIMULATION_MODE
    const uint16_t phase = packet->sequence % 20U;
    packet->joystick_left_x = phase < 5U ? 2048U : (phase < 10U ? 3200U : 900U);
    packet->joystick_left_y = phase < 5U ? 3200U : 2048U;
    packet->joystick_right_x = (uint16_t)(1600U + ((phase * 97U) % 900U));
    packet->joystick_right_y = (uint16_t)(1500U + ((phase * 73U) % 1000U));
    packet->buttons = phase == 3U ? 0x0001U : (phase == 12U ? 0x0024U : 0U);
    packet->battery_level = (uint16_t)(3900U - (packet->sequence % 200U));
#else
    int value;
    uint16_t *values[] = {&packet->joystick_left_x, &packet->joystick_left_y,
                          &packet->joystick_right_x, &packet->joystick_right_y,
                          &packet->battery_level};
    for (size_t i = 0; i < sizeof(adc_channels) / sizeof(adc_channels[0]); ++i) {
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, adc_channels[i], &value));
        *values[i] = (uint16_t)value;
    }
    packet->buttons = read_buttons();
#endif
    packet->movement_command = (uint8_t)control_packet_command_from_joystick(
        packet->joystick_left_x, packet->joystick_left_y);
    control_packet_finalize(packet);
}
