// THIS IS AUTOMATICALLY GENERATED CODE. DO NOT MODIFY.

using System;
using System.Collections.Generic;
using System.Linq;

namespace DataLink
{
    public class datalink_message
    {
        public byte msg_id { get; set; }
        public byte len { get; set; }
        public byte[] payload { get; set; }

        public datalink_message()
        {
            this.msg_id = 0;
            this.len = 0;
            this.payload = null;
        }

        public datalink_message(byte msgId, byte[] payload)
        {
            this.msg_id = msgId;
            this.len = (byte)(payload != null ? payload.Length : 0);
            this.payload = payload;
        }

        public byte[] Serialize()
        {
            var data = new byte[2 + (payload != null ? payload.Length : 0)];

            data[0] = msg_id;
            data[1] = len;

            if (payload != null)
            {
                Array.Copy(payload, 0, data, 2, payload.Length);
            }

            return data;
        }

        public static datalink_message Deserialize(byte[] data)
        {
            if (data.Length < 2)
            {
                return null;
            }

            var message = new datalink_message();
            message.msg_id = data[0];
            message.len = data[1];

            if (message.len != data.Length - 2)
            {
                return null;
            }

            message.payload = new byte[message.len];
            Array.Copy(data, 2, message.payload, 0, message.len);

            return message;
        }
    };

    public static class DataLinkMemoryUtils
    {
        static DataLinkMemoryUtils()
        {
            if (!BitConverter.IsLittleEndian)
            {
                throw new NotSupportedException("Only little-endian architectures are supported.");
            }
        }

        public static void WriteUint8(byte[] buffer, int offset, byte value)
        {
            buffer[offset] = value;
        }

        public static void WriteUint16(byte[] buffer, int offset, ushort value)
        {
            BitConverter.GetBytes(value).CopyTo(buffer, offset);
        }

        public static void WriteUint32(byte[] buffer, int offset, uint value)
        {
            BitConverter.GetBytes(value).CopyTo(buffer, offset);
        }

        public static void WriteInt8(byte[] buffer, int offset, sbyte value)
        {
            buffer[offset] = (byte)value;
        }

        public static void WriteInt16(byte[] buffer, int offset, short value)
        {
            BitConverter.GetBytes(value).CopyTo(buffer, offset);
        }

        public static void WriteInt32(byte[] buffer, int offset, int value)
        {
            BitConverter.GetBytes(value).CopyTo(buffer, offset);
        }

        public static void WriteFloat(byte[] buffer, int offset, float value)
        {
            BitConverter.GetBytes(value).CopyTo(buffer, offset);
        }

        public static void WriteDouble(byte[] buffer, int offset, double value)
        {
            BitConverter.GetBytes(value).CopyTo(buffer, offset);
        }

        public static byte ReadUint8(byte[] buffer, int offset)
        {
            return buffer[offset];
        }

        public static ushort ReadUint16(byte[] buffer, int offset)
        {
            return BitConverter.ToUInt16(buffer, offset);
        }

        public static uint ReadUint32(byte[] buffer, int offset)
        {
            return BitConverter.ToUInt32(buffer, offset);
        }

        public static sbyte ReadInt8(byte[] buffer, int offset)
        {
            return (sbyte)buffer[offset];
        }

        public static short ReadInt16(byte[] buffer, int offset)
        {
            return BitConverter.ToInt16(buffer, offset);
        }

        public static int ReadInt32(byte[] buffer, int offset)
        {
            return BitConverter.ToInt32(buffer, offset);
        }

        public static float ReadFloat(byte[] buffer, int offset)
        {
            return BitConverter.ToSingle(buffer, offset);
        }

        public static double ReadDouble(byte[] buffer, int offset)
        {
            return BitConverter.ToDouble(buffer, offset);
        }
    }

    // {{{DATA}}}

    public static class DataLinkSerial
    {
        public const byte DATALINK_MAGIC_SERIAL = 0x7E;

        private struct frame_structure
        {
            public byte magic_serial;
            public byte msg_id;
            public byte len;
            public byte[] payload;
            public ushort crc;
        }

        public static byte[] Serialize(datalink_message frame)
        {
            byte[] buffer = new byte[3 + (frame.payload != null ? frame.payload.Length : 0) + 2];
            int offset = 0;

            buffer[offset++] = DATALINK_MAGIC_SERIAL;
            buffer[offset++] = frame.msg_id;

            if (frame.payload != null)
            {
                buffer[offset++] = frame.len;

                for (var i = 0; i < frame.payload.Length; i++)
                {
                    buffer[offset++] = frame.payload[i];
                }
            }
            else
            {
                buffer[offset++] = 0;
            }

            ushort crc = CalculateCRC16(buffer, offset);

            buffer[offset++] = (byte)(crc & 0xff);
            buffer[offset++] = (byte)(crc >> 8);

            var encodedData = COBS_Encode(buffer);

            encodedData = encodedData.Concat(new byte[] { 0x00 }).ToArray();

            return encodedData;
        }

        public static datalink_message Deserialize(byte[] data)
        {
            data = data[..^1];

            byte[] decodedData = COBS_Decode(data);

            if (decodedData.Length < 5)
            {
                return null;
            }

            var frame = new frame_structure();

            frame.magic_serial = decodedData[0];

            if (frame.magic_serial != DATALINK_MAGIC_SERIAL)
            {
                return null;
            }

            frame.crc |= decodedData[decodedData.Length - 2];
            frame.crc |= (ushort)(decodedData[decodedData.Length - 1] << 8);

            ushort crc = CalculateCRC16(decodedData, decodedData.Length - 2);

            if (crc != frame.crc)
            {
                return null;
            }

            frame.msg_id = decodedData[1];
            frame.len = decodedData[2];

            if (frame.len != decodedData.Length - 5)
            {
                return null;
            }

            if (frame.len > 0)
            {
                frame.payload = new byte[frame.len];

                for (var i = 0; i < frame.len; i++)
                {
                    frame.payload[i] = decodedData[3 + i];
                }
            }
            else
            {
                frame.payload = null;
            }

            return new datalink_message(frame.msg_id, frame.payload);
        }

        public static byte[] COBS_Encode(byte[] data)
        {
            var list = new List<byte>();
            var code = (byte)0x01;
            var lastIndex = 0;

            list.Add(0x00);

            for (var i = 0; i < data.Length; i++)
            {
                var b = data[i];

                if (b > 0x00)
                {
                    list.Add(b);
                    code++;
                }

                if (b == 0x00 || code == 0xff)
                {
                    list[lastIndex] = code;
                    code = 1;

                    list.Add(0x00);
                    lastIndex = list.Count - 1;
                }
            }

            list[lastIndex] = code;

            return list.ToArray();
        }

        public static byte[] COBS_Decode(byte[] data)
        {
            var list = new List<byte>();
            var code = (byte)0xff;
            var block = 0;

            for (var i = 0; i < data.Length; i++)
            {
                var b = data[i];

                if (block > 0)
                {
                    list.Add(b);
                }
                else
                {
                    if (code != 0xff)
                    {
                        list.Add(0x00);
                    }

                    code = b;
                    block = code;

                    if (code == 0x00)
                    {
                        break;
                    }
                }

                block--;
            }

            return list.ToArray();
        }

        public static ushort CalculateCRC16(byte[] data, int len)
        {
            if (data.Length < len)
            {
                return 0x0000;
            }

            ushort crc = 0xffff;
            byte t, L;

            for (int i = 0; i < len; i++)
            {
                crc ^= data[i];
                L = (byte)(crc ^ (crc << 4));
                t = (byte)((L << 3) | (L >> 5));
                L ^= (byte)(t & 0x07);
                t = (byte)((t & 0xF8) ^ (((t << 1) | (t >> 7)) & 0x0F) ^ (byte)(crc >> 8));
                crc = (ushort)((L << 8) | t);
            }

            return crc;
        }
    }
}