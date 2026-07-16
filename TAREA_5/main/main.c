#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "app_config.h"
#include "calibration.h"
#include "control_input.h"
#include "control_packet.h"
#include "display.h"
#include "imu_mpu6050.h"
#include "mqtt_manager.h"
#include "pin_config.h"
#include "rf433.h"
#include "wifi_manager.h"

static const char *TAG = "control_main";

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_LOGI(TAG, "TAREA_5 starting; ESP32-S3 simulation=%d", CONTROL_SIMULATION_MODE);
    calibration_t calibration;
    calibration_init(&calibration);
    control_input_init();
    imu_mpu6050_init();
    display_init();
    wifi_manager_init();
    mqtt_manager_init();
    rf433_init();

    uint32_t sequence = 0;
    bool mqtt_started = false;
    TickType_t last_mqtt = 0;
    TickType_t last_display = 0;
    while (true) {
        if (!mqtt_started && wifi_manager_is_connected()) {
            mqtt_manager_start();
            mqtt_started = true;
        }
        control_input_state_t input;
        imu_state_t imu;
        control_input_read(&input, &calibration.joystick);
        imu_mpu6050_read(&imu, &calibration.imu);
        calibration_update(&calibration, &input, &imu);
        display_handle_input(&input);
        control_packet_t packet;
        control_packet_build(&packet, sequence++, &input, &imu, &calibration);
        rf433_send_packet(&packet);
        const TickType_t now = xTaskGetTickCount();
        const bool buttons_changed = input.pressed_edges || input.released_edges;
        if (buttons_changed || now - last_mqtt >= pdMS_TO_TICKS(APP_MQTT_PERIOD_MS)) {
            mqtt_manager_publish(&packet, buttons_changed);
            last_mqtt = now;
        }
        if (now - last_display >= pdMS_TO_TICKS(APP_DISPLAY_PERIOD_MS)) {
            display_update(&packet, &calibration, wifi_manager_is_connected(), mqtt_manager_is_connected(),
                           wifi_manager_ip_address());
            last_display = now;
        }
        vTaskDelay(pdMS_TO_TICKS(APP_SAMPLE_PERIOD_MS));
    }
}
