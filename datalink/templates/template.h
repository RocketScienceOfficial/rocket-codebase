// THIS IS AUTOMATICALLY GENERATED CODE. DO NOT MODIFY.

#ifndef _DATALINK_H
#define _DATALINK_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DATALINK_OK 0
#define DATALINK_ERROR 1

#define DATALINK_MESSAGE_MAX_PAYLOAD_SIZE 254

typedef struct 
{
    uint8_t msg_id;
    uint8_t len;
    uint8_t payload[DATALINK_MESSAGE_MAX_PAYLOAD_SIZE];
} datalink_message_t;

uint16_t datalink_crc16_mcrf4xx_calculate(const uint8_t *data, int length);

int datalink_cobs_encode(const uint8_t *data, int length, uint8_t *buffer, int buffer_size);
int datalink_cobs_decode(const uint8_t *buffer, int length, uint8_t *data, int data_size);

int datalink_serialize_message(const datalink_message_t *msg, uint8_t *buffer, int *len);
int datalink_deserialize_message(datalink_message_t *msg, const uint8_t *buffer, int len);
int datalink_serialize_message_serial(const datalink_message_t *msg, uint8_t *buffer, int *len);
int datalink_deserialize_message_serial(datalink_message_t *msg, const uint8_t *buffer, int len);
int datalink_serialize_message_radio(const datalink_message_t *msg, uint8_t seq, uint8_t src_id, uint8_t dest_id, uint8_t *buffer, int *len);
int datalink_deserialize_message_radio(datalink_message_t *msg, uint8_t *seq, uint8_t *src_id, uint8_t *dest_id, const uint8_t *buffer, int len);

// {{{DATA}}}

#ifdef __cplusplus
}
#endif

#endif // _DATALINK_H
