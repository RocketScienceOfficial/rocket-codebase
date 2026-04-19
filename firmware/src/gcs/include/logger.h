#pragma once

#include "config.h"
#include <Arduino.h>

#define IS_DEBUG 0

#if IS_DEBUG
#define SERIAL_DEBUG_PRINTF(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
#else
#define SERIAL_DEBUG_PRINTF(fmt, ...)
#endif