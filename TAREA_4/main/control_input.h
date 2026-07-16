#pragma once

#include "control_packet.h"

void control_input_init(void);
void control_input_read(control_packet_t *packet);
