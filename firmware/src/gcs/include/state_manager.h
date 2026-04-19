#pragma once

#define BUTTON_PIN 38

#define SYSTEM_STATES_COUNT 3

enum class SystemState
{
    GCS,
    ROCKET,
    OTHER,
};

void state_init();
void state_update();
bool state_changed();
SystemState state_get_current();