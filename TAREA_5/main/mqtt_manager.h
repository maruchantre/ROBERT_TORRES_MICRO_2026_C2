#pragma once

#include <stdbool.h>

#include "control_packet.h"

void mqtt_manager_init(void);
void mqtt_manager_start(void);
void mqtt_manager_publish(const control_packet_t *packet, bool buttons_changed);
bool mqtt_manager_is_connected(void);
