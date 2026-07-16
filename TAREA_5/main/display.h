#pragma once

#include "calibration.h"
#include "control_packet.h"

void display_init(void);
void display_handle_input(const control_input_state_t *input);
void display_update(const control_packet_t *packet, const calibration_t *calibration,
                    bool wifi_connected, bool mqtt_connected, const char *ip_address);
