#ifndef _WAVEFORM_UTILS_PICO_H
#define _WAVEFORM_UTILS_PICO_H

#include "hal/waveform_driver.h"
#include "hardware/pio.h"

/** @brief Creates a waveform channel for the Pico platform */
#define HAL_WAVEFORM_PICO_CHANNEL_CREATE(pio, sm) ((hal_waveform_channel_t)(((uint8_t)(pio) << 4) | ((uint8_t)(sm) & 0x0F)))

/** @brief Gets the PIO instance from a Pico waveform channel */
#define HAL_WAVEFORM_PICO_CHANNEL_GET_PIO(channel) ((PIO)(((channel) >> 4) == 0 ? pio0 : pio1))

/** @brief Gets the state machine from a Pico waveform channel */
#define HAL_WAVEFORM_PICO_CHANNEL_GET_SM(channel) ((uint8_t)((channel) & 0x0F))

#endif