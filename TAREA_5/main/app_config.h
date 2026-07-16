#pragma once

#include "pin_config.h"

#define APP_DEVICE_NAME "kakata433"
#define APP_MQTT_BROKER_URI "mqtt://broker.emqx.io:1883"
#define APP_MQTT_TOPIC_PREFIX "itla/kakata433/maruchantre_7f29"
#define APP_SAMPLE_PERIOD_MS 20
#define APP_MQTT_PERIOD_MS 200
#define APP_DISPLAY_PERIOD_MS 250
#define APP_ADC_SAMPLE_COUNT 8
#define APP_ADC_FILTER_SHIFT 2
#define APP_JOYSTICK_DEAD_ZONE 5
#define APP_GYRO_DEAD_ZONE 3
#define APP_GYRO_FULL_SCALE_DPS 250.0f
#define APP_CALIBRATION_HOLD_MS 3000
#define APP_CALIBRATION_SAMPLES 100
#define APP_CALIBRATION_LED PIN_LED1

/* OLED controller, resolution and address are undocumented in this repository. */
#define APP_OLED_AVAILABLE 0
