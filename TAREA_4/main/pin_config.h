#pragma once

#include "driver/gpio.h"

/* Set to 0 to read the KACATA-RC433-V1 board. */
#define CONTROL_SIMULATION_MODE 1

/* Analog inputs, all on ADC unit 1 of the ESP32-S3. */
#define PIN_ADC_JOY1_MD GPIO_NUM_1
#define PIN_ADC_JOY1_MT GPIO_NUM_2
#define PIN_ADC_JOY0_MT GPIO_NUM_4
#define PIN_ADC_JOY0_MD GPIO_NUM_5
#define PIN_ADC_VBAT    GPIO_NUM_8

/* Buttons are active-low: the switch connects the signal to GND. */
#define PIN_JOY0_BTN GPIO_NUM_3
#define PIN_BTN_0    GPIO_NUM_9
#define PIN_BTN_2    GPIO_NUM_10
#define PIN_BTN_1    GPIO_NUM_11
#define PIN_BTN_3    GPIO_NUM_12
#define PIN_BTNL4    GPIO_NUM_39
#define PIN_BTNL3    GPIO_NUM_40
#define PIN_BTNL2    GPIO_NUM_41
#define PIN_BTNL1    GPIO_NUM_42
#define PIN_JOY1_BTN GPIO_NUM_46

#define PIN_I2C_SDA GPIO_NUM_6
#define PIN_I2C_SCL GPIO_NUM_7

#define PIN_RF_ENABLE        GPIO_NUM_15
#define PIN_DATA_RF_SEND     GPIO_NUM_17
#define PIN_DATA_RF_RECEIVED GPIO_NUM_18
#define PIN_MPU_INT          GPIO_NUM_16

/* U5 is a 74HC04 inverter, so these LEDs are active-low at the ESP32-S3. */
#define PIN_LED6 GPIO_NUM_13
#define PIN_LED5 GPIO_NUM_14
#define PIN_LED4 GPIO_NUM_21
#define PIN_LED1 GPIO_NUM_45
#define PIN_LED3 GPIO_NUM_47
#define PIN_LED2 GPIO_NUM_48
