idf_component_register(SRCS "main.c" "control_input.c" "control_packet.c" "rf433.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_adc esp_driver_gpio esp_driver_i2c)
