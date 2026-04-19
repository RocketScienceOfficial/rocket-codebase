#pragma once

#include <datalink.h>

#define SERIAL_BAUD_RATE 115200

void serial_init();
void serial_update();
void serial_send_frame(const datalink_message_t *msg);