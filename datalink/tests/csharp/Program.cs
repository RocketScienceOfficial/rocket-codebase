using System;
using System.Linq;
using DataLink;

internal static class Program
{
    private static int _failed;

    private static int Main()
    {
        Run(nameof(Test_Datalink_Message_Roundtrip), Test_Datalink_Message_Roundtrip);
        Run(nameof(Test_Message_Deserialize_Rejects_Short_Data), Test_Message_Deserialize_Rejects_Short_Data);
        Run(nameof(Test_Message_Deserialize_Rejects_Len_Mismatch), Test_Message_Deserialize_Rejects_Len_Mismatch);
        Run(nameof(Test_COBS_Roundtrip), Test_COBS_Roundtrip);
        Run(nameof(Test_COBS_Known_Vector), Test_COBS_Known_Vector);
        Run(nameof(Test_COBS_Empty_Input), Test_COBS_Empty_Input);
        Run(nameof(Test_COBS_Decode_Empty_Input), Test_COBS_Decode_Empty_Input);
        Run(nameof(Test_CRC16_Known_Vector), Test_CRC16_Known_Vector);
        Run(nameof(Test_Serial_Roundtrip), Test_Serial_Roundtrip);
        Run(nameof(Test_Serial_Rejects_Invalid_Magic), Test_Serial_Rejects_Invalid_Magic);
        Run(nameof(Test_Serial_Rejects_CRC_Mismatch), Test_Serial_Rejects_CRC_Mismatch);
        Run(nameof(Test_Serial_Rejects_Declared_Len_Mismatch), Test_Serial_Rejects_Declared_Len_Mismatch);

        Console.WriteLine();
        Console.WriteLine($"Passed: {11 - _failed}, Failed: {_failed}");

        return _failed == 0 ? 0 : 1;
    }

    private static void Run(string name, Action test)
    {
        try
        {
            test();
            Console.WriteLine($"[PASS] {name}");
        }
        catch (Exception ex)
        {
            _failed++;
            Console.WriteLine($"[FAIL] {name}: {ex.Message}");
        }
    }

    private static void AssertTrue(bool condition, string message)
    {
        if (!condition)
        {
            throw new InvalidOperationException(message);
        }
    }

    private static void AssertBytesEqual(byte[] actual, byte[] expected, string message)
    {
        if (!actual.SequenceEqual(expected))
        {
            throw new InvalidOperationException(message);
        }
    }

    private static datalink_message CreateMessage(byte msgId, byte[] payload)
    {
        var raw = new byte[2 + payload.Length];
        raw[0] = msgId;
        raw[1] = (byte)payload.Length;
        Array.Copy(payload, 0, raw, 2, payload.Length);

        var msg = datalink_message.Deserialize(raw);
        if (msg == null)
        {
            throw new InvalidOperationException("Unable to construct datalink_message test value.");
        }

        return msg;
    }

    private static byte[] RebuildSerialFrame(byte[] decoded)
    {
        var data = DataLinkSerial.COBS_Encode(decoded);
        data = data.Concat(new byte[] { 0x00 }).ToArray();
        return data;
    }

    private static byte[] DecodeSerialFrame(byte[] serialized)
    {
        serialized = serialized[..^1];
        var decoded = DataLinkSerial.COBS_Decode(serialized);
        return decoded;
    }

    private static void Test_Datalink_Message_Roundtrip()
    {
        var message = CreateMessage(7, new byte[] { 0x01, 0x02, 0x03 });
        var serialized = message.Serialize();
        var parsed = datalink_message.Deserialize(serialized);

        AssertTrue(parsed != null, "Parsed message must not be null");
        AssertTrue(parsed!.msg_id == 7, "Message ID mismatch");
        AssertTrue(parsed.len == 3, "Message length mismatch");
        AssertBytesEqual(parsed.payload, new byte[] { 0x01, 0x02, 0x03 }, "Message payload mismatch");
    }

    private static void Test_Message_Deserialize_Rejects_Short_Data()
    {
        var parsed = datalink_message.Deserialize(new byte[] { 0x01 });
        AssertTrue(parsed == null, "Expected null for short data");
    }

    private static void Test_Message_Deserialize_Rejects_Len_Mismatch()
    {
        var parsed = datalink_message.Deserialize(new byte[] { 0x01, 0x03, 0xAA });
        AssertTrue(parsed == null, "Expected null for header length mismatch");
    }

    private static void Test_COBS_Roundtrip()
    {
        var payloads = new[]
        {
            Array.Empty<byte>(),
            new byte[] { 0x00 },
            new byte[] { 0x61, 0x62, 0x63 },
            new byte[] { 0x00, 0x01, 0x00, 0x02, 0x03, 0x00 },
            Enumerable.Range(1, 63).Select(i => (byte)i).ToArray()
        };

        foreach (var payload in payloads)
        {
            var encoded = DataLinkSerial.COBS_Encode(payload);
            var decoded = DataLinkSerial.COBS_Decode(encoded);
            AssertBytesEqual(decoded, payload, "COBS roundtrip mismatch");
        }
    }

    private static void Test_COBS_Known_Vector()
    {
        var encoded = DataLinkSerial.COBS_Encode(new byte[] { 0x11, 0x22, 0x00, 0x33 });
        AssertBytesEqual(encoded, new byte[] { 0x03, 0x11, 0x22, 0x02, 0x33 }, "COBS known vector mismatch");
    }

    private static void Test_COBS_Empty_Input()
    {
        var encoded = DataLinkSerial.COBS_Encode(Array.Empty<byte>());
        var decoded = DataLinkSerial.COBS_Decode(new byte[] { 0x01 });

        AssertBytesEqual(encoded, new byte[] { 0x01 }, "COBS empty encode mismatch");
        AssertBytesEqual(decoded, Array.Empty<byte>(), "COBS empty decode mismatch");
    }

    private static void Test_COBS_Decode_Empty_Input()
    {
        var decoded = DataLinkSerial.COBS_Decode(new byte[] { });

        AssertBytesEqual(decoded, Array.Empty<byte>(), "COBS empty decode mismatch");
    }

    private static void Test_CRC16_Known_Vector()
    {
        var data = new byte[] { 0x01, 0x02, 0x03, 0x04 };
        var crc = DataLinkSerial.CalculateCRC16(data, data.Length);
        AssertTrue(crc == 0xC66E, "CRC known vector mismatch");
    }

    private static void Test_Serial_Roundtrip()
    {
        var original = CreateMessage(4, new byte[] { 0x10, 0x20, 0x30 });
        var serialized = DataLinkSerial.Serialize(original);
        var parsed = DataLinkSerial.Deserialize(serialized);

        AssertTrue(parsed != null, "Parsed serial message must not be null");
        AssertTrue(parsed!.msg_id == original.msg_id, "Serial message ID mismatch");
        AssertBytesEqual(parsed.payload, original.payload, "Serial payload mismatch");
    }

    private static void Test_Serial_Rejects_Invalid_Magic()
    {
        var serialized = DataLinkSerial.Serialize(CreateMessage(1, new byte[] { 0xAA, 0xBB }));
        var decoded = DecodeSerialFrame(serialized);
        decoded[0] ^= 0x01;
        var tampered = RebuildSerialFrame(decoded);

        var parsed = DataLinkSerial.Deserialize(tampered);
        AssertTrue(parsed == null, "Expected null for invalid serial magic");
    }

    private static void Test_Serial_Rejects_CRC_Mismatch()
    {
        var serialized = DataLinkSerial.Serialize(CreateMessage(2, new byte[] { 0x11, 0x22, 0x33 }));
        var decoded = DecodeSerialFrame(serialized);
        decoded[3] ^= 0x01;
        var tampered = RebuildSerialFrame(decoded);

        var parsed = DataLinkSerial.Deserialize(tampered);
        AssertTrue(parsed == null, "Expected null for CRC mismatch");
    }

    private static void Test_Serial_Rejects_Declared_Len_Mismatch()
    {
        var serialized = DataLinkSerial.Serialize(CreateMessage(3, new byte[] { 0x99, 0x88 }));
        var decoded = DecodeSerialFrame(serialized);
        decoded[2] = (byte)(decoded[2] + 1);
        var tampered = RebuildSerialFrame(decoded);

        var parsed = DataLinkSerial.Deserialize(tampered);
        AssertTrue(parsed == null, "Expected null for declared payload length mismatch");
    }
}