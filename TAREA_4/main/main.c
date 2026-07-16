#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "control_input.h"
#include "control_packet.h"
#include "pin_config.h"
#include "rf433.h"

static const char *TAG = "control_main";

void app_main(void)
{
    ESP_LOGI(TAG, "KACATA RC433 control starting; simulation=%d", CONTROL_SIMULATION_MODE);
    control_input_init();
    rf433_init();
    while (true) {
        control_packet_t packet = {0};
        control_input_read(&packet);
        rf433_send_packet(&packet);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
