#include "radio_controller.h"
#include "serial_controller.h"
#include "gps_controller.h"
#include "commander.h"
#include "maths.h"
#include "logger.h"
#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <string.h>

#define DEVICE_ID 0xDF
#define OBC_ID 0x11
#define RADIO_TLM_DATA_SEND_DELAY 100
#define TEMP_RX_RESET_TIME 5000

static RadioData g_current_data;
static uint8_t g_current_seq;
static int g_packets_lost;
static int g_tmp_rx;
static unsigned long g_send_tlm_data_time_offset;
static unsigned long g_tmp_rx_time_offset;

static void _try_parse_packet(uint8_t *buffer, size_t len);
static void _send_tlm_packet();

void radio_init()
{
    SERIAL_DEBUG_PRINTF("Starting LoRa...\n");

    SPI.begin(LORA_SCLK_PIN, LORA_MISO_PIN, LORA_MOSI_PIN);
    LoRa.setPins(LORA_CS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);

    if (!LoRa.begin(LORA_FREQ))
    {
        SERIAL_DEBUG_PRINTF("Starting LoRa failed!\n");

        while (true)
            ;
    }

    LoRa.setFrequency(LORA_FREQ);
    LoRa.setSignalBandwidth(LORA_BAND);
    LoRa.setSpreadingFactor(LORA_SF);
    LoRa.receive();

    SERIAL_DEBUG_PRINTF("Starting LoRa success!\n");
}

void radio_update()
{
    int packetSize = LoRa.parsePacket();

    if (packetSize)
    {
        SERIAL_DEBUG_PRINTF("Received packet with size: %d bytes\n", packetSize);

        uint8_t buffer[512];
        size_t i = 0;

        while (LoRa.available())
        {
            if (i == packetSize)
            {
                SERIAL_DEBUG_PRINTF("Something went wrong while parsing packet!\n");

                return;
            }

            if (i < sizeof(buffer))
            {
                buffer[i++] = (uint8_t)LoRa.read();
            }
            else
            {
                SERIAL_DEBUG_PRINTF("Buffer overflow while parsing packet!\n");

                return;
            }
        }

        _try_parse_packet(buffer, i);
    }

    if (g_send_tlm_data_time_offset != 0 && millis() - g_send_tlm_data_time_offset >= RADIO_TLM_DATA_SEND_DELAY)
    {
        _send_tlm_packet();

        g_send_tlm_data_time_offset = 0;
    }

    if (millis() - g_tmp_rx_time_offset >= TEMP_RX_RESET_TIME)
    {
        g_tmp_rx = 0;
        g_packets_lost = 0;
        g_tmp_rx_time_offset = millis();
    }
}

const RadioData &radio_get_data()
{
    return g_current_data;
}

void _try_parse_packet(uint8_t *buffer, size_t len)
{
    datalink_message_t msg;
    uint8_t seq, srcId, destId;

    if (datalink_deserialize_message_radio(&msg, &seq, &srcId, &destId, buffer, len) != DATALINK_OK)
    {
        SERIAL_DEBUG_PRINTF("Couldn't deserialize packet!\n");

        return;
    }

    if (srcId != OBC_ID || destId != DEVICE_ID)
    {
        SERIAL_DEBUG_PRINTF("Packet source or destination is invalid!\n");

        return;
    }

    g_current_data.rssi = LoRa.packetRssi();
    g_current_data.rx++;
    g_tmp_rx = g_current_data.rx;

    if (seq != g_current_seq)
    {
        g_packets_lost += seq > g_current_seq ? seq - g_current_seq : 256 + seq - g_current_seq;
        g_current_seq = seq + 1;
    }
    else if (g_current_seq == 255)
    {
        g_current_seq = 0;
    }
    else
    {
        g_current_seq++;
    }

    if (msg.msg_id != DATALINK_MESSAGE_ID_TELEMETRY_DATA_OBC)
    {
        SERIAL_DEBUG_PRINTF("Invalid message ID!\n");

        return;
    }

    datalink_unpack_telemetry_data_obc(&g_current_data.frame, &msg);

    commander_new_frame(g_current_data.frame.cmd_seq, g_current_data.frame.cmd_last_status);

    telemetry_data_gcs newPayload;
    newPayload.qw = g_current_data.frame.qw;
    newPayload.qx = g_current_data.frame.qx;
    newPayload.qy = g_current_data.frame.qy;
    newPayload.qz = g_current_data.frame.qz;
    newPayload.velocity_kmh = g_current_data.frame.velocity_kmh;
    newPayload.batteryVoltage100 = g_current_data.frame.batteryVoltage100;
    newPayload.batteryPercentage = g_current_data.frame.batteryPercentage;
    newPayload.lat = g_current_data.frame.lat;
    newPayload.lon = g_current_data.frame.lon;
    newPayload.alt = g_current_data.frame.alt;
    newPayload.gcsLat = (int)(gps_get_data().latitude * 10000000);
    newPayload.gcsLon = (int)(gps_get_data().longitude * 10000000);
    newPayload.gpsData = g_current_data.frame.gpsData;
    newPayload.state = g_current_data.frame.state;
    newPayload.stateFlags = g_current_data.frame.stateFlags;
    newPayload.signalStrengthNeg = (uint8_t)-g_current_data.rssi;
    newPayload.packetLossPercentage = (uint8_t)((float)g_packets_lost / (g_tmp_rx + g_packets_lost) * 100);
    newPayload.packetsReceived = (uint16_t)g_current_data.rx;
    newPayload.packetsTransmitted = (uint16_t)g_current_data.tx;

    datalink_pack_telemetry_data_gcs(&newPayload, &msg);
    serial_send_frame(&msg);

    if (g_current_data.frame.sendResponse == 1)
    {
        g_send_tlm_data_time_offset = millis();
    }

    SERIAL_DEBUG_PRINTF("Successfully parsed packet!\n");
}

void _send_tlm_packet()
{
    static uint8_t sequence = 0;

    uint8_t buffer[512];
    int len = sizeof(buffer);

    telemetry_response payload;
    payload.cmd = commander_get_current_cmd();
    payload.cmd_seq = commander_get_current_seq();

    datalink_message_t msg;
    datalink_pack_telemetry_response(&payload, &msg);

    if (datalink_serialize_message_radio(&msg, sequence, DEVICE_ID, OBC_ID, buffer, &len) == DATALINK_OK)
    {
        LoRa.beginPacket();
        LoRa.write(buffer, len);
        LoRa.endPacket();

        g_current_data.tx++;

        SERIAL_DEBUG_PRINTF("Successfully sent %d bytes\n", len);

        LoRa.receive();
    }
    else
    {
        SERIAL_DEBUG_PRINTF("Couldn't serialize packet to send!\n");
    }

    sequence = sequence == 255 ? 0 : sequence + 1;
}