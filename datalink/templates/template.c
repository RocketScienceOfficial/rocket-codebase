// THIS IS AUTOMATICALLY GENERATED CODE. DO NOT MODIFY.

#include "template.h"
#include <string.h>

#ifndef __cplusplus
    #define static_assert _Static_assert
#endif

static_assert(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__, "Error: System is not Little-Endian!");

static int _safe_copy(uint8_t *buffer, int *bufferOffset, int bufferLen, const uint8_t *src, int srcLen)
{
    if (srcLen + *bufferOffset <= bufferLen)
    {
        if (srcLen > 0)
        {
            memcpy(buffer + *bufferOffset, src, srcLen);

            *bufferOffset += srcLen;
        }

        return 1;
    }
    else
    {
        return 0;
    }
}

uint16_t crc16_mcrf4xx_table_internal[256] = {
    0x0000, 0x1189, 0x2312, 0x329B, 0x4624, 0x57AD, 0x6536, 0x74BF,
    0x8C48, 0x9DC1, 0xAF5A, 0xBED3, 0xCA6C, 0xDBE5, 0xE97E, 0xF8F7,
    0x1081, 0x0108, 0x3393, 0x221A, 0x56A5, 0x472C, 0x75B7, 0x643E,
    0x9CC9, 0x8D40, 0xBFDB, 0xAE52, 0xDAED, 0xCB64, 0xF9FF, 0xE876,
    0x2102, 0x308B, 0x0210, 0x1399, 0x6726, 0x76AF, 0x4434, 0x55BD,
    0xAD4A, 0xBCC3, 0x8E58, 0x9FD1, 0xEB6E, 0xFAE7, 0xC87C, 0xD9F5,
    0x3183, 0x200A, 0x1291, 0x0318, 0x77A7, 0x662E, 0x54B5, 0x453C,
    0xBDCB, 0xAC42, 0x9ED9, 0x8F50, 0xFBEF, 0xEA66, 0xD8FD, 0xC974,
    0x4204, 0x538D, 0x6116, 0x709F, 0x0420, 0x15A9, 0x2732, 0x36BB,
    0xCE4C, 0xDFC5, 0xED5E, 0xFCD7, 0x8868, 0x99E1, 0xAB7A, 0xBAF3,
    0x5285, 0x430C, 0x7197, 0x601E, 0x14A1, 0x0528, 0x37B3, 0x263A,
    0xDECD, 0xCF44, 0xFDDF, 0xEC56, 0x98E9, 0x8960, 0xBBFB, 0xAA72,
    0x6306, 0x728F, 0x4014, 0x519D, 0x2522, 0x34AB, 0x0630, 0x17B9,
    0xEF4E, 0xFEC7, 0xCC5C, 0xDDD5, 0xA96A, 0xB8E3, 0x8A78, 0x9BF1,
    0x7387, 0x620E, 0x5095, 0x411C, 0x35A3, 0x242A, 0x16B1, 0x0738,
    0xFFCF, 0xEE46, 0xDCDD, 0xCD54, 0xB9EB, 0xA862, 0x9AF9, 0x8B70,
    0x8408, 0x9581, 0xA71A, 0xB693, 0xC22C, 0xD3A5, 0xE13E, 0xF0B7,
    0x0840, 0x19C9, 0x2B52, 0x3ADB, 0x4E64, 0x5FED, 0x6D76, 0x7CFF,
    0x9489, 0x8500, 0xB79B, 0xA612, 0xD2AD, 0xC324, 0xF1BF, 0xE036,
    0x18C1, 0x0948, 0x3BD3, 0x2A5A, 0x5EE5, 0x4F6C, 0x7DF7, 0x6C7E,
    0xA50A, 0xB483, 0x8618, 0x9791, 0xE32E, 0xF2A7, 0xC03C, 0xD1B5,
    0x2942, 0x38CB, 0x0A50, 0x1BD9, 0x6F66, 0x7EEF, 0x4C74, 0x5DFD,
    0xB58B, 0xA402, 0x9699, 0x8710, 0xF3AF, 0xE226, 0xD0BD, 0xC134,
    0x39C3, 0x284A, 0x1AD1, 0x0B58, 0x7FE7, 0x6E6E, 0x5CF5, 0x4D7C,
    0xC60C, 0xD785, 0xE51E, 0xF497, 0x8028, 0x91A1, 0xA33A, 0xB2B3,
    0x4A44, 0x5BCD, 0x6956, 0x78DF, 0x0C60, 0x1DE9, 0x2F72, 0x3EFB,
    0xD68D, 0xC704, 0xF59F, 0xE416, 0x90A9, 0x8120, 0xB3BB, 0xA232,
    0x5AC5, 0x4B4C, 0x79D7, 0x685E, 0x1CE1, 0x0D68, 0x3FF3, 0x2E7A,
    0xE70E, 0xF687, 0xC41C, 0xD595, 0xA12A, 0xB0A3, 0x8238, 0x93B1,
    0x6B46, 0x7ACF, 0x4854, 0x59DD, 0x2D62, 0x3CEB, 0x0E70, 0x1FF9,
    0xF78F, 0xE606, 0xD49D, 0xC514, 0xB1AB, 0xA022, 0x92B9, 0x8330,
    0x7BC7, 0x6A4E, 0x58D5, 0x495C, 0x3DE3, 0x2C6A, 0x1EF1, 0x0F78
};

uint16_t datalink_crc16_mcrf4xx_calculate(const uint8_t *data, int length)
{
    uint16_t crc = 0xFFFF; // MCRF4XX Initial value
    
    while (length--) 
    {
        crc = (crc >> 8) ^ crc16_mcrf4xx_table_internal[(crc ^ *data++) & 0xFF];
    }

    return crc;
}

/**
 * REF: https://rosettacode.org/wiki/Consistent_overhead_byte_stuffing
 * 
 * @brief Encodes the input data using Consistent Overhead Byte Stuffing (COBS). Output buffer does not include the trailing zero byte, which should be added by the caller if needed.
 * @return Returns the number of bytes written to data, or -1 if an error occurred (e.g., data_size is too small).
 */
int datalink_cobs_encode(const uint8_t *data, int length, uint8_t *buffer, int buffer_size)
{
    uint8_t *encode = buffer;
    uint8_t *codep = encode++;
    uint8_t code = 1;

    for (const uint8_t *byte = data; length--; ++byte)
    {
        if (*byte)
        {
            if ((int)(encode - buffer) >= buffer_size)
            {
                return -1;
            }

            *encode++ = *byte;
            ++code;
        }

        if (!*byte || code == 0xff)
        {
            *codep = code;
            code = 1;
            codep = encode;

            if (!*byte || length)
            {
                if ((int)(encode - buffer) >= buffer_size)
                {
                    return -1;
                }

                ++encode;
            }
        }
    }

    *codep = code;

    return (int)(encode - buffer);
}

/**
 * REF: https://rosettacode.org/wiki/Consistent_overhead_byte_stuffing
 * 
 * @brief Decodes the input data using Consistent Overhead Byte Stuffing (COBS). Trailing zero byte is not expected in the input buffer.
 * @return Returns the number of bytes written to data, or -1 if an error occurred (e.g., data_size is too small).
 */
int datalink_cobs_decode(const uint8_t *buffer, int length, uint8_t *data, int data_size)
{
    const uint8_t *byte = buffer;
    uint8_t *decode = data;

    for (uint8_t code = 0xff, block = 0; byte < buffer + length; --block)
    {
        if (block)
        {
            if ((int)(decode - data) >= data_size)
            {
                return -1;
            }

            *decode++ = *byte++;
        }
        else
        {
            if (code != 0xff)
            {
                if ((int)(decode - data) >= data_size)
                {
                    return -1;
                }

                *decode++ = 0;
            }

            block = code = *byte++;

            if (!code)
            {
                break;
            }
        }
    }

    return (int)(decode - data);
}

int datalink_serialize_message(const datalink_message_t *msg, uint8_t *buffer, int *len)
{
    if (*len < msg->len + 2)
    {
        return DATALINK_ERROR;
    }

    int offset = 0;

    buffer[offset++] = msg->msg_id;
    buffer[offset++] = msg->len;

    if (!_safe_copy(buffer, &offset, *len, msg->payload, msg->len))
    {
        return DATALINK_ERROR;
    }

    *len = offset;

    return DATALINK_OK;
}

int datalink_deserialize_message(datalink_message_t *msg, const uint8_t *buffer, int len)
{
    if (len < 2)
    {
        return DATALINK_ERROR;
    }

    int offset = 0;

    msg->msg_id = buffer[offset++];
    msg->len = buffer[offset++];

    if (len != msg->len + offset || msg->len > DATALINK_MESSAGE_MAX_PAYLOAD_SIZE)
    {
        return DATALINK_ERROR;
    }

    memcpy(msg->payload, buffer + 2, msg->len);

    return DATALINK_OK;
}

#define DATALINK_MAGIC_SERIAL 0x7E

typedef struct datalink_frame_structure_serial
{
    uint8_t magic_serial;
    uint8_t msgId;
    uint8_t len;
    uint8_t payload[DATALINK_MESSAGE_MAX_PAYLOAD_SIZE];
    uint16_t crc;
} datalink_frame_structure_serial_t;

int datalink_serialize_message_serial(const datalink_message_t *msg, uint8_t *buffer, int *len)
{
    uint8_t tmpBuffer[3 + DATALINK_MESSAGE_MAX_PAYLOAD_SIZE + 2];
    int tmpBufferLen = sizeof(tmpBuffer);
    int offset = 0;

    tmpBuffer[offset++] = DATALINK_MAGIC_SERIAL;
    tmpBuffer[offset++] = msg->msg_id;
    tmpBuffer[offset++] = msg->len;

    if (!_safe_copy(tmpBuffer, &offset, tmpBufferLen, msg->payload, msg->len))
    {
        return DATALINK_ERROR;
    }

    uint16_t crc = datalink_crc16_mcrf4xx_calculate(tmpBuffer, offset);

    if (!_safe_copy(tmpBuffer, &offset, tmpBufferLen, (const uint8_t *)&crc, 2))
    {
        return DATALINK_ERROR;
    }

    int cobs_len = datalink_cobs_encode(tmpBuffer, offset, buffer, *len);

    if (cobs_len < 0 || cobs_len >= *len)
    {
        return DATALINK_ERROR;
    }

    buffer[cobs_len++] = 0x00;
    *len = cobs_len;

    return DATALINK_OK;
}

int datalink_deserialize_message_serial(datalink_message_t *msg, const uint8_t *buffer, int len)
{
    uint8_t tmpBuffer[3 + DATALINK_MESSAGE_MAX_PAYLOAD_SIZE + 2];
    int tmpBufferLen = sizeof(tmpBuffer);

    if (len < 1)
    {
        return DATALINK_ERROR;
    }

    tmpBufferLen = datalink_cobs_decode(buffer, len - 1, tmpBuffer, tmpBufferLen);

    if (tmpBufferLen < 5)
    {
        return DATALINK_ERROR;
    }

    datalink_frame_structure_serial_t frame;

    frame.magic_serial = tmpBuffer[0];

    if (frame.magic_serial != DATALINK_MAGIC_SERIAL)
    {
        return DATALINK_ERROR;
    }

    memcpy(&frame.crc, tmpBuffer + tmpBufferLen - 2, 2);

    uint16_t crc = datalink_crc16_mcrf4xx_calculate(tmpBuffer, tmpBufferLen - 2);

    if (frame.crc != crc)
    {
        return DATALINK_ERROR;
    }

    msg->msg_id = tmpBuffer[1];
    msg->len = tmpBuffer[2];

    if (msg->len != tmpBufferLen - 5 || msg->len > DATALINK_MESSAGE_MAX_PAYLOAD_SIZE)
    {
        return DATALINK_ERROR;
    }

    if (msg->len > 0)
    {
        memcpy(msg->payload, tmpBuffer + 3, msg->len);
    }

    return DATALINK_OK;
}

#define DATALINK_MAGIC_RADIO 0x5A

typedef struct datalink_frame_structure_radio
{
    uint8_t magic_radio;
    uint8_t seq;
    uint8_t src_id;
    uint8_t dest_id;
    uint8_t msg_id;
    uint8_t len;
    uint8_t payload[DATALINK_MESSAGE_MAX_PAYLOAD_SIZE];
    uint16_t crc;
} datalink_frame_structure_radio_t;

int datalink_serialize_message_radio(const datalink_message_t *msg, uint8_t seq, uint8_t src_id, uint8_t dest_id, uint8_t *buffer, int *len)
{
    int offset = 0;

    if (*len >= 6)
    {
        buffer[offset++] = DATALINK_MAGIC_RADIO;
        buffer[offset++] = seq;
        buffer[offset++] = src_id;
        buffer[offset++] = dest_id;
        buffer[offset++] = msg->msg_id;
        buffer[offset++] = msg->len;
    }
    else
    {
        return DATALINK_ERROR;
    }

    if (!_safe_copy(buffer, &offset, *len, msg->payload, msg->len))
    {
        return DATALINK_ERROR;
    }

    uint16_t crc = datalink_crc16_mcrf4xx_calculate(buffer, offset);

    if (!_safe_copy(buffer, &offset, *len, (const uint8_t *)&crc, 2))
    {
        return DATALINK_ERROR;
    }

    *len = offset;

    return DATALINK_OK;
}

int datalink_deserialize_message_radio(datalink_message_t *msg, uint8_t *seq, uint8_t *src_id, uint8_t *dest_id, const uint8_t *buffer, int len)
{
    if (len < 8)
    {
        return DATALINK_ERROR;
    }

    datalink_frame_structure_radio_t frame;

    frame.magic_radio = buffer[0];

    if (frame.magic_radio != DATALINK_MAGIC_RADIO)
    {
        return DATALINK_ERROR;
    }

    memcpy(&frame.crc, buffer + len - 2, 2);

    uint16_t crc = datalink_crc16_mcrf4xx_calculate(buffer, len - 2);

    if (frame.crc != crc)
    {
        return DATALINK_ERROR;
    }

    *seq = buffer[1];
    *src_id = buffer[2];
    *dest_id = buffer[3];

    msg->msg_id = buffer[4];
    msg->len = buffer[5];

    if (msg->len != len - 8 || msg->len > DATALINK_MESSAGE_MAX_PAYLOAD_SIZE)
    {
        return DATALINK_ERROR;
    }

    if (msg->len > 0)
    {
        memcpy(msg->payload, buffer + 6, msg->len);
    }

    return DATALINK_OK;
}

// {{{DATA}}}