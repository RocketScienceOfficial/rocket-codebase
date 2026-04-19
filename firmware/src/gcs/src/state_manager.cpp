#include "state_manager.h"
#include "logger.h"
#include <Arduino.h>

static int g_current_state_index = 0;
static int g_last_state_index = 0;
static int g_button_state = LOW;
static bool g_button_pressed = false;
static bool g_initialized = false;

void state_init()
{
    pinMode(BUTTON_PIN, INPUT);

    SERIAL_DEBUG_PRINTF("Initialized state!\n");
}

void state_update()
{
    g_button_state = digitalRead(BUTTON_PIN);

    g_last_state_index = g_current_state_index;

    if (g_button_state == HIGH)
    {
        if (!g_initialized)
        {
            return;
        }

        if (!g_button_pressed)
        {
            g_current_state_index++;

            if (g_current_state_index >= SYSTEM_STATES_COUNT)
            {
                g_current_state_index = 0;
            }

            SERIAL_DEBUG_PRINTF("State changed\n");

            g_button_pressed = true;
        }
    }
    else
    {
        g_button_pressed = false;
        g_initialized = true;
    }
}

bool state_changed()
{
    return g_current_state_index != g_last_state_index;
}

SystemState state_get_current()
{
    return (SystemState)g_current_state_index;
}