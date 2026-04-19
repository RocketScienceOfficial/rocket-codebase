#include "commander.h"
#include "serial_controller.h"
#include "logger.h"
#include <datalink.h>
#include <Arduino.h>

static uint8_t g_current_cmd;
static uint8_t g_current_cmd_seq;
static bool g_cmd_active;
static unsigned long g_last_cmd_time_offset;
static uint8_t g_current_remote_seq;

static void _reset_cmd()
{
    g_current_cmd = DATALINK_TELEMETRY_CMD_NONE;
    g_cmd_active = false;
    g_last_cmd_time_offset = 0;
}

static void _ack(bool success)
{
    _reset_cmd();

    gcs_ack ack;
    ack.success = (uint8_t)success;

    datalink_message_t msg;
    datalink_pack_gcs_ack(&ack, &msg);
    serial_send_frame(&msg);

    SERIAL_DEBUG_PRINTF("Successfully acknowledged command!\n");
}

static void _nack()
{
    g_current_cmd_seq = g_current_remote_seq;

    _reset_cmd();

    datalink_message_t msg;
    datalink_pack_gcs_nack(&msg);
    serial_send_frame(&msg);

    SERIAL_DEBUG_PRINTF("Successfully sent negative acknowledgment!\n");
}

void commander_set_cmd(uint8_t cmd)
{
    if (g_cmd_active)
    {
        SERIAL_DEBUG_PRINTF("Cannot set new command while another command is active!\n");
        return;
    }

    g_current_cmd = cmd;
    g_current_cmd_seq = (g_current_cmd_seq + 1) % 256;
    g_last_cmd_time_offset = millis();
    g_cmd_active = true;

    SERIAL_DEBUG_PRINTF("Set command to %d with sequence number %d\n", cmd, g_current_cmd_seq);
}

void commander_new_frame(uint8_t seq, uint8_t status)
{
    g_current_remote_seq = seq;

    SERIAL_DEBUG_PRINTF("Remote Seq: %d Status: %d Current: %d\n", g_current_remote_seq, status, g_current_cmd_seq);

    if (g_cmd_active && g_current_remote_seq == g_current_cmd_seq)
    {
        if (status == DATALINK_TELEMETRY_CMD_STATUS_PENDING)
        {
            g_current_cmd = DATALINK_TELEMETRY_CMD_NONE;
        }
        else
        {
            _ack(status == DATALINK_TELEMETRY_CMD_STATUS_SUCCESS);
        }
    }
    else if (!g_cmd_active && g_current_remote_seq != g_current_cmd_seq)
    {
        _nack();
    }
}

void commander_update()
{
    if (g_cmd_active)
    {
        if (millis() - g_last_cmd_time_offset >= CMD_TIMEOUT)
        {
            _nack();
        }
    }
}

uint8_t commander_get_current_cmd()
{
    return g_current_cmd;
}

uint8_t commander_get_current_seq()
{
    return g_current_cmd_seq;
}

uint8_t commander_get_current_timeout_left()
{
    if (!g_cmd_active)
    {
        return 0;
    }

    unsigned long elapsed = millis() - g_last_cmd_time_offset;

    if (elapsed >= CMD_TIMEOUT)
    {
        return 0;
    }

    return (uint8_t)((CMD_TIMEOUT - elapsed) / 1000);
}