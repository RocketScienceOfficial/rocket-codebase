#include "serial_controller.h"
#include "radio_controller.h"
#include "commander.h"
#include "logger.h"
#include <Arduino.h>

static uint8_t g_buffer[512];
static size_t g_current_size;

static void _process_new_message(const datalink_message_t &msg)
{
    if (msg.msg_id == DATALINK_MESSAGE_ID_TELEMETRY_GCS_CMD)
    {
        telemetry_gcs_cmd payload;
        datalink_unpack_telemetry_gcs_cmd(&payload, &msg);

        commander_set_cmd(payload.cmd);
    }
}

void serial_init()
{
    Serial.begin(SERIAL_BAUD_RATE);

    SERIAL_DEBUG_PRINTF("Initialized serial port!\n");
}

void serial_update()
{
    if (Serial.available())
    {
        int c = Serial.read();

        if (g_current_size >= sizeof(g_buffer))
        {
            g_current_size = 0;
        }

        g_buffer[g_current_size++] = (uint8_t)c;

        if (c == 0x00)
        {
            datalink_message_t msg;

            if (datalink_deserialize_message_serial(&msg, g_buffer, g_current_size) == DATALINK_OK)
            {
                _process_new_message(msg);
            }

            g_current_size = 0;
        }
    }
}

void serial_send_frame(const datalink_message_t *msg)
{
    uint8_t buffer[512];
    int len = sizeof(buffer);

    if (datalink_serialize_message_serial(msg, buffer, &len) == DATALINK_OK)
    {
        Serial.write(buffer, len);
        Serial.flush();
    }
}