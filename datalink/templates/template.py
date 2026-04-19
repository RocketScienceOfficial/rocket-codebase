# THIS IS AUTOMATICALLY GENERATED CODE. DO NOT MODIFY.

import sys
import struct
from dataclasses import dataclass


# Fail immediately upon importing if the system is not little-endian
if sys.byteorder != 'little':
    raise SystemError("FATAL: This protocol library strictly requires a Little-Endian system.")


class datalink_message:
    def __init__(self, msg_id: int, payload: bytes):
        self.msg_id = msg_id
        self.len = len(payload)
        self.payload = payload

    def serialize(self) -> bytes:
        return struct.pack('<BB', self.msg_id, self.len) + self.payload

    @staticmethod
    def deserialize(data: bytes) -> 'datalink_message':
        if len(data) < 2:
            raise ValueError('Data too short to be a valid datalink_message')

        msg_id, length = struct.unpack('<BB', data[:2])

        if len(data) != 2 + length:
            raise ValueError('Data length does not match length specified in header')

        payload = data[2:2+length]

        return datalink_message(msg_id, payload)


class datalink_utils:

    @staticmethod
    def crc16(data: bytes) -> int:
        crc = 0xffff

        for b in data:
            crc ^= b
            L = (crc ^ (crc << 4)) & 0xFF
            t = ((L << 3) & 0xFF) | (L >> 5)
            L ^= (t & 0x07)
            L &= 0xFF
            t_rotated = ((t << 1) | (t >> 7)) & 0x0F
            t = ((t & 0xF8) ^ t_rotated ^ ((crc >> 8) & 0xFF)) & 0xFF
            crc = ((L << 8) | t) & 0xFFFF

        return crc

    @staticmethod
    def cobs_encode(data: bytes) -> bytes:
        if not data:
            return b"\x01"

        out = bytearray()
        idx = 0

        while idx <= len(data):
            block_start = idx
            # Find next zero or max block length (254 bytes)
            while idx < len(data) and data[idx] != 0 and (idx - block_start) < 254:
                idx += 1

            block_len = idx - block_start
            out.append(block_len + 1)
            out.extend(data[block_start:idx])

            if idx < len(data) and data[idx] == 0:
                idx += 1  # skip the zero
            else:
                # If we're at the end AND last byte was zero,
                # we must emit the final empty block
                if idx == len(data):
                    break

        return bytes(out)

    @staticmethod
    def cobs_decode(data: bytes) -> bytes:
        if not data:
            return b""

        out = bytearray()
        idx = 0

        while idx < len(data):
            code = data[idx]
            if code == 0:
                raise ValueError("Invalid COBS data (zero byte found)")

            idx += 1
            end = idx + code - 1

            if end > len(data):
                raise ValueError("Invalid COBS data (overflow)")

            out.extend(data[idx:end])
            idx = end

            # Insert zero if not last block
            if code != 0xFF and idx < len(data):
                out.append(0)

        return bytes(out)


class datalink_serial:
    MAGIC = 0x7E

    class frame_structure:
        magic_serial: int
        msg_id: int
        len: int
        payload: bytes
        crc: int

    @staticmethod
    def serialize(msg: datalink_message) -> bytes:
        buff = struct.pack('<B', datalink_serial.MAGIC) + msg.serialize()
        crc = datalink_utils.crc16(buff)
        final = buff + struct.pack('<H', crc)
        encoded = datalink_utils.cobs_encode(final)
        encoded += b'\x00'

        return encoded

    @staticmethod
    def deserialize(data: bytes) -> datalink_message:
        data.pop()
        decoded = datalink_utils.cobs_decode(data)

        if len(decoded) < 5:
            raise ValueError('Decoded data too short to be a valid datalink_serial frame')

        frame = datalink_serial.frame_structure()
        frame.magic_serial, frame.msg_id, frame.len = struct.unpack('<BBB', decoded[:3])
        frame.payload = decoded[3:-2]
        frame.crc = struct.unpack('<H', decoded[-2:])[0]

        if frame.magic_serial != datalink_serial.MAGIC:
            raise ValueError('Invalid magic byte in datalink_serial frame')
        if frame.len != len(frame.payload):
            print(f"Expected payload length: {frame.len}, actual payload length: {len(frame.payload)}")
            raise ValueError('Decoded data length does not match length specified in frame header')
        if datalink_utils.crc16(decoded[:-2]) != frame.crc:
            raise ValueError('CRC mismatch in datalink_serial frame')

        return datalink_message(frame.msg_id, frame.payload)


# {{{DATA}}}
