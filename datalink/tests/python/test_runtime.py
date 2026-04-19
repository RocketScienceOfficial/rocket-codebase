import pytest
import datalink


def rebuild_serial_frame(decoded_frame: bytes) -> bytes:
    return datalink.datalink_utils.cobs_encode(decoded_frame) + b"\x00"


def decode_serial_frame(encoded_frame: bytes) -> bytes:
    # The serial frame is COBS encoded and terminated by a zero byte.
    return datalink.datalink_utils.cobs_decode(encoded_frame[:-1])


def test_datalink_message_roundtrip():
    message = datalink.datalink_message(7, b"\x01\x02\x03")
    serialized = message.serialize()
    deserialized = datalink.datalink_message.deserialize(serialized)

    assert deserialized.msg_id == 7
    assert deserialized.len == 3
    assert deserialized.payload == b"\x01\x02\x03"


@pytest.mark.parametrize(
    "payload",
    [
        b"",
        b"\x00",
        b"abc",
        bytes([0, 1, 0, 2, 3, 0]),
        bytes(range(1, 64)),
    ],
)
def test_cobs_roundtrip(payload):
    encoded = datalink.datalink_utils.cobs_encode(payload)
    decoded = datalink.datalink_utils.cobs_decode(encoded)

    assert decoded == payload


def test_cobs_encode():
    assert datalink.datalink_utils.cobs_encode(b"\x11\x22\x00\x33") == b"\x03\x11\x22\x02\x33"


def test_cobs_decode():
    assert datalink.datalink_utils.cobs_decode(b"\x03\x11\x22\x02\x33") == b"\x11\x22\x00\x33"


def test_cobs_encode_empty_input():
    assert datalink.datalink_utils.cobs_encode(b"") == b"\x01"


def test_cobs_decode_almost_empty_input():
    assert datalink.datalink_utils.cobs_decode(b"\x01") == b""


def test_cobs_decode_empty_input():
    assert datalink.datalink_utils.cobs_decode(b"") == b""


def test_crc16_known_vector():
    data = b"\x01\x02\x03\x04"
    expected_crc = 0xC66E

    assert datalink.datalink_utils.crc16(data) == expected_crc


def test_message_deserialize_rejects_short_data():
    with pytest.raises(ValueError, match="Data too short"):
        datalink.datalink_message.deserialize(b"\x01")


def test_message_deserialize_rejects_len_mismatch():
    with pytest.raises(ValueError, match="Data length does not match"):
        datalink.datalink_message.deserialize(b"\x01\x03\xAA")


def test_serial_roundtrip():
    original = datalink.datalink_message(4, b"\x10\x20\x30")

    serialized = datalink.datalink_serial.serialize(original)
    parsed = datalink.datalink_serial.deserialize(bytearray(serialized))

    assert parsed.msg_id == original.msg_id
    assert parsed.payload == original.payload


def test_serial_rejects_invalid_magic():
    serialized = datalink.datalink_serial.serialize(datalink.datalink_message(1, b"\xAA\xBB"))
    decoded = bytearray(decode_serial_frame(serialized))
    decoded[0] ^= 0x01
    tampered = rebuild_serial_frame(bytes(decoded))

    with pytest.raises(ValueError, match="Invalid magic"):
        datalink.datalink_serial.deserialize(bytearray(tampered))


def test_serial_rejects_crc_mismatch():
    serialized = datalink.datalink_serial.serialize(datalink.datalink_message(2, b"\x11\x22\x33"))
    decoded = bytearray(decode_serial_frame(serialized))

    # Flip one payload byte but keep original CRC to guarantee mismatch.
    decoded[3] ^= 0x01
    tampered = rebuild_serial_frame(bytes(decoded))

    with pytest.raises(ValueError, match="CRC mismatch"):
        datalink.datalink_serial.deserialize(bytearray(tampered))


def test_serial_rejects_declared_len_mismatch():
    serialized = datalink.datalink_serial.serialize(datalink.datalink_message(3, b"\x99\x88"))
    decoded = bytearray(decode_serial_frame(serialized))

    # Byte 2 is declared payload length in frame structure.
    decoded[2] = decoded[2] + 1
    tampered = rebuild_serial_frame(bytes(decoded))

    with pytest.raises(ValueError, match="Decoded data length does not match"):
        datalink.datalink_serial.deserialize(bytearray(tampered))
