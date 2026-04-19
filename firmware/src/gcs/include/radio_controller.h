#pragma once

#include <datalink.h>

#define LORA_FREQ 433000000
#define LORA_BAND 250000
#define LORA_SF 7
#define LORA_SCLK_PIN 5
#define LORA_MISO_PIN 19
#define LORA_MOSI_PIN 27
#define LORA_CS_PIN 18
#define LORA_DIO0_PIN 26
#define LORA_RST_PIN 23

struct RadioData
{
    int rx;
    int tx;
    int rssi;
    telemetry_data_obc frame;
};

void radio_init();
void radio_update();
const RadioData &radio_get_data();