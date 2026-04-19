#pragma once

#include <stdint.h>

#define CMD_TIMEOUT 10000

void commander_set_cmd(uint8_t cmd);
void commander_new_frame(uint8_t seq, uint8_t status);
void commander_update();

uint8_t commander_get_current_cmd();
uint8_t commander_get_current_seq();
uint8_t commander_get_current_timeout_left();