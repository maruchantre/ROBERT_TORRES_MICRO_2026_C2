#pragma once

#include "control_packet.h"

void rf433_init(void);
void rf433_send_packet(const control_packet_t *packet);
