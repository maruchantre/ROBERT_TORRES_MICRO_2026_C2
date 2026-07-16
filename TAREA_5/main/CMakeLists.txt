idf_component_register(SRCS "main.c" "control_input.c" "control_packet.c" "calibration.c"
                            "imu_mpu6050.c" "display.c" "wifi_manager.c" "mqtt_manager.c" "rf433.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_adc esp_driver_gpio esp_driver_i2c esp_wifi esp_event esp_netif mqtt nvs_flash)
