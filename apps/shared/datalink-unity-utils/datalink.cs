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

    public enum state_machine_state : byte
{
    DATALINK_SM_STATE_STANDING = 0,
    DATALINK_SM_STATE_ARMED = 1,
    DATALINK_SM_STATE_ACCELERATING = 2,
    DATALINK_SM_STATE_FREE_FLIGHT = 3,
    DATALINK_SM_STATE_FREE_FALL = 4,
    DATALINK_SM_STATE_LANDED = 5,
};

public enum data_chunk_ign_flags : byte
{
    DATALINK_FLAGS_IGN_1_CONT = 1,
    DATALINK_FLAGS_IGN_2_CONT = 2,
    DATALINK_FLAGS_IGN_3_CONT = 4,
    DATALINK_FLAGS_IGN_4_CONT = 8,
    DATALINK_FLAGS_IGN_1_FIRED = 16,
    DATALINK_FLAGS_IGN_2_FIRED = 32,
    DATALINK_FLAGS_IGN_3_FIRED = 64,
    DATALINK_FLAGS_IGN_4_FIRED = 128,
};

public enum sitl_read_flags : byte
{
    DATALINK_FLAGS_SITL_READ_IMU_1 = 1,
    DATALINK_FLAGS_SITL_READ_MAG_1 = 2,
    DATALINK_FLAGS_SITL_READ_BARO_1 = 4,
    DATALINK_FLAGS_SITL_READ_GPS_1 = 8,
};

public enum telemetry_data_state_flags : byte
{
    DATALINK_FLAGS_TELEMETRY_DATA_CONTROL_3V3_ENABLED = 1,
    DATALINK_FLAGS_TELEMETRY_DATA_CONTROL_5V_ENABLED = 2,
    DATALINK_FLAGS_TELEMETRY_DATA_CONTROL_VBAT_ENABLED = 4,
    DATALINK_FLAGS_TELEMETRY_DATA_CONTROL_IGN_1_CONT = 8,
    DATALINK_FLAGS_TELEMETRY_DATA_CONTROL_IGN_2_CONT = 16,
    DATALINK_FLAGS_TELEMETRY_DATA_CONTROL_IGN_3_CONT = 32,
    DATALINK_FLAGS_TELEMETRY_DATA_CONTROL_IGN_4_CONT = 64,
};

public enum telemetry_cmd : byte
{
    DATALINK_TELEMETRY_CMD_NONE = 0,
    DATALINK_TELEMETRY_CMD_ARM = 1,
    DATALINK_TELEMETRY_CMD_DISARM = 2,
    DATALINK_TELEMETRY_CMD_3V3_ENABLED = 3,
    DATALINK_TELEMETRY_CMD_3V3_DISABLED = 4,
    DATALINK_TELEMETRY_CMD_5V_ENABLED = 5,
    DATALINK_TELEMETRY_CMD_5V_DISABLED = 6,
    DATALINK_TELEMETRY_CMD_VBAT_ENABLED = 7,
    DATALINK_TELEMETRY_CMD_VBAT_DISABLED = 8,
    DATALINK_TELEMETRY_CMD_IGN_1_REQ_FIRE = 9,
    DATALINK_TELEMETRY_CMD_IGN_2_REQ_FIRE = 10,
    DATALINK_TELEMETRY_CMD_IGN_3_REQ_FIRE = 11,
    DATALINK_TELEMETRY_CMD_IGN_4_REQ_FIRE = 12,
};

public enum telemetry_cmd_status : byte
{
    DATALINK_TELEMETRY_CMD_STATUS_PENDING = 0,
    DATALINK_TELEMETRY_CMD_STATUS_SUCCESS = 1,
    DATALINK_TELEMETRY_CMD_STATUS_FAILURE = 2,
};

public class saved_data_chunk_obc
{
    public const int MSG_ID = 10;
    public const int MSG_SIZE = 111;

    public double lat { get; set; }
    public double lon { get; set; }
    public double alt { get; set; }
    public float accRawX { get; set; }
    public float accRawY { get; set; }
    public float accRawZ { get; set; }
    public float gyroRawX { get; set; }
    public float gyroRawY { get; set; }
    public float gyroRawZ { get; set; }
    public float magRawX { get; set; }
    public float magRawY { get; set; }
    public float magRawZ { get; set; }
    public int pressure { get; set; }
    public float velN { get; set; }
    public float velE { get; set; }
    public float velD { get; set; }
    public float posN { get; set; }
    public float posE { get; set; }
    public float posD { get; set; }
    public float qw { get; set; }
    public float qx { get; set; }
    public float qy { get; set; }
    public float qz { get; set; }
    public ushort dt { get; set; }
    public ushort batteryVoltage100 { get; set; }
    public byte gpsData { get; set; } // First bit - GPS fix, rest - sats number
    public byte smState { get; set; } // Based on state_machine_state
    public byte ignFlags { get; set; } // Flags from data_chunk_ign_flags

    public datalink_message Pack()
    {
        byte[] payload = new byte[MSG_SIZE];

        DataLinkMemoryUtils.WriteDouble(payload, 0, this.lat);
        DataLinkMemoryUtils.WriteDouble(payload, 8, this.lon);
        DataLinkMemoryUtils.WriteDouble(payload, 16, this.alt);
        DataLinkMemoryUtils.WriteFloat(payload, 24, this.accRawX);
        DataLinkMemoryUtils.WriteFloat(payload, 28, this.accRawY);
        DataLinkMemoryUtils.WriteFloat(payload, 32, this.accRawZ);
        DataLinkMemoryUtils.WriteFloat(payload, 36, this.gyroRawX);
        DataLinkMemoryUtils.WriteFloat(payload, 40, this.gyroRawY);
        DataLinkMemoryUtils.WriteFloat(payload, 44, this.gyroRawZ);
        DataLinkMemoryUtils.WriteFloat(payload, 48, this.magRawX);
        DataLinkMemoryUtils.WriteFloat(payload, 52, this.magRawY);
        DataLinkMemoryUtils.WriteFloat(payload, 56, this.magRawZ);
        DataLinkMemoryUtils.WriteInt32(payload, 60, this.pressure);
        DataLinkMemoryUtils.WriteFloat(payload, 64, this.velN);
        DataLinkMemoryUtils.WriteFloat(payload, 68, this.velE);
        DataLinkMemoryUtils.WriteFloat(payload, 72, this.velD);
        DataLinkMemoryUtils.WriteFloat(payload, 76, this.posN);
        DataLinkMemoryUtils.WriteFloat(payload, 80, this.posE);
        DataLinkMemoryUtils.WriteFloat(payload, 84, this.posD);
        DataLinkMemoryUtils.WriteFloat(payload, 88, this.qw);
        DataLinkMemoryUtils.WriteFloat(payload, 92, this.qx);
        DataLinkMemoryUtils.WriteFloat(payload, 96, this.qy);
        DataLinkMemoryUtils.WriteFloat(payload, 100, this.qz);
        DataLinkMemoryUtils.WriteUint16(payload, 104, this.dt);
        DataLinkMemoryUtils.WriteUint16(payload, 106, this.batteryVoltage100);
        DataLinkMemoryUtils.WriteUint8(payload, 108, this.gpsData);
        DataLinkMemoryUtils.WriteUint8(payload, 109, this.smState);
        DataLinkMemoryUtils.WriteUint8(payload, 110, this.ignFlags);

        return new datalink_message(MSG_ID, payload);
    }

    public static saved_data_chunk_obc Unpack(datalink_message message)
    {
        if (message.msg_id != MSG_ID || message.len != MSG_SIZE)
        {
            return null;
        }

        saved_data_chunk_obc frame = new saved_data_chunk_obc();

        frame.lat = DataLinkMemoryUtils.ReadDouble(message.payload, 0);
        frame.lon = DataLinkMemoryUtils.ReadDouble(message.payload, 8);
        frame.alt = DataLinkMemoryUtils.ReadDouble(message.payload, 16);
        frame.accRawX = DataLinkMemoryUtils.ReadFloat(message.payload, 24);
        frame.accRawY = DataLinkMemoryUtils.ReadFloat(message.payload, 28);
        frame.accRawZ = DataLinkMemoryUtils.ReadFloat(message.payload, 32);
        frame.gyroRawX = DataLinkMemoryUtils.ReadFloat(message.payload, 36);
        frame.gyroRawY = DataLinkMemoryUtils.ReadFloat(message.payload, 40);
        frame.gyroRawZ = DataLinkMemoryUtils.ReadFloat(message.payload, 44);
        frame.magRawX = DataLinkMemoryUtils.ReadFloat(message.payload, 48);
        frame.magRawY = DataLinkMemoryUtils.ReadFloat(message.payload, 52);
        frame.magRawZ = DataLinkMemoryUtils.ReadFloat(message.payload, 56);
        frame.pressure = DataLinkMemoryUtils.ReadInt32(message.payload, 60);
        frame.velN = DataLinkMemoryUtils.ReadFloat(message.payload, 64);
        frame.velE = DataLinkMemoryUtils.ReadFloat(message.payload, 68);
        frame.velD = DataLinkMemoryUtils.ReadFloat(message.payload, 72);
        frame.posN = DataLinkMemoryUtils.ReadFloat(message.payload, 76);
        frame.posE = DataLinkMemoryUtils.ReadFloat(message.payload, 80);
        frame.posD = DataLinkMemoryUtils.ReadFloat(message.payload, 84);
        frame.qw = DataLinkMemoryUtils.ReadFloat(message.payload, 88);
        frame.qx = DataLinkMemoryUtils.ReadFloat(message.payload, 92);
        frame.qy = DataLinkMemoryUtils.ReadFloat(message.payload, 96);
        frame.qz = DataLinkMemoryUtils.ReadFloat(message.payload, 100);
        frame.dt = DataLinkMemoryUtils.ReadUint16(message.payload, 104);
        frame.batteryVoltage100 = DataLinkMemoryUtils.ReadUint16(message.payload, 106);
        frame.gpsData = DataLinkMemoryUtils.ReadUint8(message.payload, 108);
        frame.smState = DataLinkMemoryUtils.ReadUint8(message.payload, 109);
        frame.ignFlags = DataLinkMemoryUtils.ReadUint8(message.payload, 110);

        return frame;
    }
}

public class saved_data_chunk_acs
{
    public const int MSG_ID = 11;
    public const int MSG_SIZE = 115;

    public double lat { get; set; }
    public double lon { get; set; }
    public double alt { get; set; }
    public float accRawX { get; set; }
    public float accRawY { get; set; }
    public float accRawZ { get; set; }
    public float gyroRawX { get; set; }
    public float gyroRawY { get; set; }
    public float gyroRawZ { get; set; }
    public float magRawX { get; set; }
    public float magRawY { get; set; }
    public float magRawZ { get; set; }
    public int pressure { get; set; }
    public float velN { get; set; }
    public float velE { get; set; }
    public float velD { get; set; }
    public float posN { get; set; }
    public float posE { get; set; }
    public float posD { get; set; }
    public float qw { get; set; }
    public float qx { get; set; }
    public float qy { get; set; }
    public float qz { get; set; }
    public ushort dt { get; set; }
    public ushort batteryVoltage100 { get; set; }
    public short angleSetpoint10 { get; set; }
    public short pidRoll10 { get; set; }
    public byte gpsData { get; set; } // First bit - GPS fix, rest - sats number
    public byte smState { get; set; } // Based on state_machine_state
    public sbyte pidOutputAngle10 { get; set; }

    public datalink_message Pack()
    {
        byte[] payload = new byte[MSG_SIZE];

        DataLinkMemoryUtils.WriteDouble(payload, 0, this.lat);
        DataLinkMemoryUtils.WriteDouble(payload, 8, this.lon);
        DataLinkMemoryUtils.WriteDouble(payload, 16, this.alt);
        DataLinkMemoryUtils.WriteFloat(payload, 24, this.accRawX);
        DataLinkMemoryUtils.WriteFloat(payload, 28, this.accRawY);
        DataLinkMemoryUtils.WriteFloat(payload, 32, this.accRawZ);
        DataLinkMemoryUtils.WriteFloat(payload, 36, this.gyroRawX);
        DataLinkMemoryUtils.WriteFloat(payload, 40, this.gyroRawY);
        DataLinkMemoryUtils.WriteFloat(payload, 44, this.gyroRawZ);
        DataLinkMemoryUtils.WriteFloat(payload, 48, this.magRawX);
        DataLinkMemoryUtils.WriteFloat(payload, 52, this.magRawY);
        DataLinkMemoryUtils.WriteFloat(payload, 56, this.magRawZ);
        DataLinkMemoryUtils.WriteInt32(payload, 60, this.pressure);
        DataLinkMemoryUtils.WriteFloat(payload, 64, this.velN);
        DataLinkMemoryUtils.WriteFloat(payload, 68, this.velE);
        DataLinkMemoryUtils.WriteFloat(payload, 72, this.velD);
        DataLinkMemoryUtils.WriteFloat(payload, 76, this.posN);
        DataLinkMemoryUtils.WriteFloat(payload, 80, this.posE);
        DataLinkMemoryUtils.WriteFloat(payload, 84, this.posD);
        DataLinkMemoryUtils.WriteFloat(payload, 88, this.qw);
        DataLinkMemoryUtils.WriteFloat(payload, 92, this.qx);
        DataLinkMemoryUtils.WriteFloat(payload, 96, this.qy);
        DataLinkMemoryUtils.WriteFloat(payload, 100, this.qz);
        DataLinkMemoryUtils.WriteUint16(payload, 104, this.dt);
        DataLinkMemoryUtils.WriteUint16(payload, 106, this.batteryVoltage100);
        DataLinkMemoryUtils.WriteInt16(payload, 108, this.angleSetpoint10);
        DataLinkMemoryUtils.WriteInt16(payload, 110, this.pidRoll10);
        DataLinkMemoryUtils.WriteUint8(payload, 112, this.gpsData);
        DataLinkMemoryUtils.WriteUint8(payload, 113, this.smState);
        DataLinkMemoryUtils.WriteInt8(payload, 114, this.pidOutputAngle10);

        return new datalink_message(MSG_ID, payload);
    }

    public static saved_data_chunk_acs Unpack(datalink_message message)
    {
        if (message.msg_id != MSG_ID || message.len != MSG_SIZE)
        {
            return null;
        }

        saved_data_chunk_acs frame = new saved_data_chunk_acs();

        frame.lat = DataLinkMemoryUtils.ReadDouble(message.payload, 0);
        frame.lon = DataLinkMemoryUtils.ReadDouble(message.payload, 8);
        frame.alt = DataLinkMemoryUtils.ReadDouble(message.payload, 16);
        frame.accRawX = DataLinkMemoryUtils.ReadFloat(message.payload, 24);
        frame.accRawY = DataLinkMemoryUtils.ReadFloat(message.payload, 28);
        frame.accRawZ = DataLinkMemoryUtils.ReadFloat(message.payload, 32);
        frame.gyroRawX = DataLinkMemoryUtils.ReadFloat(message.payload, 36);
        frame.gyroRawY = DataLinkMemoryUtils.ReadFloat(message.payload, 40);
        frame.gyroRawZ = DataLinkMemoryUtils.ReadFloat(message.payload, 44);
        frame.magRawX = DataLinkMemoryUtils.ReadFloat(message.payload, 48);
        frame.magRawY = DataLinkMemoryUtils.ReadFloat(message.payload, 52);
        frame.magRawZ = DataLinkMemoryUtils.ReadFloat(message.payload, 56);
        frame.pressure = DataLinkMemoryUtils.ReadInt32(message.payload, 60);
        frame.velN = DataLinkMemoryUtils.ReadFloat(message.payload, 64);
        frame.velE = DataLinkMemoryUtils.ReadFloat(message.payload, 68);
        frame.velD = DataLinkMemoryUtils.ReadFloat(message.payload, 72);
        frame.posN = DataLinkMemoryUtils.ReadFloat(message.payload, 76);
        frame.posE = DataLinkMemoryUtils.ReadFloat(message.payload, 80);
        frame.posD = DataLinkMemoryUtils.ReadFloat(message.payload, 84);
        frame.qw = DataLinkMemoryUtils.ReadFloat(message.payload, 88);
        frame.qx = DataLinkMemoryUtils.ReadFloat(message.payload, 92);
        frame.qy = DataLinkMemoryUtils.ReadFloat(message.payload, 96);
        frame.qz = DataLinkMemoryUtils.ReadFloat(message.payload, 100);
        frame.dt = DataLinkMemoryUtils.ReadUint16(message.payload, 104);
        frame.batteryVoltage100 = DataLinkMemoryUtils.ReadUint16(message.payload, 106);
        frame.angleSetpoint10 = DataLinkMemoryUtils.ReadInt16(message.payload, 108);
        frame.pidRoll10 = DataLinkMemoryUtils.ReadInt16(message.payload, 110);
        frame.gpsData = DataLinkMemoryUtils.ReadUint8(message.payload, 112);
        frame.smState = DataLinkMemoryUtils.ReadUint8(message.payload, 113);
        frame.pidOutputAngle10 = DataLinkMemoryUtils.ReadInt8(message.payload, 114);

        return frame;
    }
}

public class saved_data_chunk_size
{
    public const int MSG_ID = 12;
    public const int MSG_SIZE = 4;

    public uint size { get; set; }

    public datalink_message Pack()
    {
        byte[] payload = new byte[MSG_SIZE];

        DataLinkMemoryUtils.WriteUint32(payload, 0, this.size);

        return new datalink_message(MSG_ID, payload);
    }

    public static saved_data_chunk_size Unpack(datalink_message message)
    {
        if (message.msg_id != MSG_ID || message.len != MSG_SIZE)
        {
            return null;
        }

        saved_data_chunk_size frame = new saved_data_chunk_size();

        frame.size = DataLinkMemoryUtils.ReadUint32(message.payload, 0);

        return frame;
    }
}

public class clear_request
{
    public const int MSG_ID = 13;
    public const int MSG_SIZE = 0;


    public datalink_message Pack()
    {
        return new datalink_message(MSG_ID, null);
    }

    public static clear_request Unpack(datalink_message message)
    {
        if (message.msg_id != MSG_ID || message.len != MSG_SIZE)
        {
            return null;
        }

        clear_request frame = new clear_request();


        return frame;
    }
}

public class clear_progress_info
{
    public const int MSG_ID = 14;
    public const int MSG_SIZE = 1;

    public byte percentage { get; set; }

    public datalink_message Pack()
    {
        byte[] payload = new byte[MSG_SIZE];

        DataLinkMemoryUtils.WriteUint8(payload, 0, this.percentage);

        return new datalink_message(MSG_ID, payload);
    }

    public static clear_progress_info Unpack(datalink_message message)
    {
        if (message.msg_id != MSG_ID || message.len != MSG_SIZE)
        {
            return null;
        }

        clear_progress_info frame = new clear_progress_info();

        frame.percentage = DataLinkMemoryUtils.ReadUint8(message.payload, 0);

        return frame;
    }
}

public class clear_finish
{
    public const int MSG_ID = 15;
    public const int MSG_SIZE = 0;


    public datalink_message Pack()
    {
        return new datalink_message(MSG_ID, null);
    }

    public static clear_finish Unpack(datalink_message message)
    {
        if (message.msg_id != MSG_ID || message.len != MSG_SIZE)
        {
            return null;
        }

        clear_finish frame = new clear_finish();


        return frame;
    }
}

public class read_request
{
    public const int MSG_ID = 16;
    public const int MSG_SIZE = 0;


    public datalink_message Pack()
    {
        return new datalink_message(MSG_ID, null);
    }

    public static read_request Unpack(datalink_message message)
    {
        if (message.msg_id != MSG_ID || message.len != MSG_SIZE)
        {
            return null;
        }

        read_request frame = new read_request();


        return frame;
    }
}

public class read_finish
{
    public const int MSG_ID = 17;
    public const int MSG_SIZE = 0;


    public datalink_message Pack()
    {
        return new datalink_message(MSG_ID, null);
    }

    public static read_finish Unpack(datalink_message message)
    {
        if (message.msg_id != MSG_ID || message.len != MSG_SIZE)
        {
            return null;
        }

        read_finish frame = new read_finish();


        return frame;
    }
}

public class recovery_request
{
    public const int MSG_ID = 18;
    public const int MSG_SIZE = 0;


    public datalink_message Pack()
    {
        return new datalink_message(MSG_ID, null);
    }

    public static recovery_request Unpack(datalink_message message)
    {
        if (message.msg_id != MSG_ID || message.len != MSG_SIZE)
        {
            return null;
        }

        recovery_request frame = new recovery_request();


        return frame;
    }
}

public class recovery_finish
{
    public const int MSG_ID = 19;
    public const int MSG_SIZE = 0;


    public datalink_message Pack()
    {
        return new datalink_message(MSG_ID, null);
    }

    public static recovery_finish Unpack(datalink_message message)
    {
        if (message.msg_id != MSG_ID || message.len != MSG_SIZE)
        {
            return null;
        }

        recovery_finish frame = new recovery_finish();


        return frame;
    }
}

public class ign_request_test
{
    public const int MSG_ID = 30;
    public const int MSG_SIZE = 1;

    public byte ignNum { get; set; }

    public datalink_message Pack()
    {
        byte[] payload = new byte[MSG_SIZE];

        DataLinkMemoryUtils.WriteUint8(payload, 0, this.ignNum);

        return new datalink_message(MSG_ID, payload);
    }

    public static ign_request_test Unpack(datalink_message message)
    {
        if (message.msg_id != MSG_ID || message.len != MSG_SIZE)
        {
            return null;
        }

        ign_request_test frame = new ign_request_test();

        frame.ignNum = DataLinkMemoryUtils.ReadUint8(message.payload, 0);

        return frame;
    }
}

public class ign_response_test
{
    public const int MSG_ID = 31;
    public const int MSG_SIZE = 0;


    public datalink_message Pack()
    {
        return new datalink_message(MSG_ID, null);
    }

    public static ign_response_test Unpack(datalink_message message)
    {
        if (message.msg_id != MSG_ID || message.len != MSG_SIZE)
        {
            return null;
        }

        ign_response_test frame = new ign_response_test();


        return frame;
    }
}

public class sitl_request_data
{
    public const int MSG_ID = 32;
    public const int MSG_SIZE = 78;

    public double gps1Lat { get; set; }
    public double gps1Lon { get; set; }
    public double gps1Alt { get; set; }
    public float imu1dt { get; set; }
    public float imu1AccX { get; set; }
    public float imu1AccY { get; set; }
    public float imu1AccZ { get; set; }
    public float imu1GyroX { get; set; }
    public float imu1GyroY { get; set; }
    public float imu1GyroZ { get; set; }
    public float mag1X { get; set; }
    public float mag1Y { get; set; }
    public float mag1Z { get; set; }
    public int baro1Pressure { get; set; }
    public float baro1Temperature { get; set; }
    public float baro1Height { get; set; }
    public byte readFlags { get; set; } // Based on sitl_read_flags
    public byte gps1Sats { get; set; }

    public datalink_message Pack()
    {
        byte[] payload = new byte[MSG_SIZE];

        DataLinkMemoryUtils.WriteDouble(payload, 0, this.gps1Lat);
        DataLinkMemoryUtils.WriteDouble(payload, 8, this.gps1Lon);
        DataLinkMemoryUtils.WriteDouble(payload, 16, this.gps1Alt);
        DataLinkMemoryUtils.WriteFloat(payload, 24, this.imu1dt);
        DataLinkMemoryUtils.WriteFloat(payload, 28, this.imu1AccX);
        DataLinkMemoryUtils.WriteFloat(payload, 32, this.imu1AccY);
        DataLinkMemoryUtils.WriteFloat(payload, 36, this.imu1AccZ);
        DataLinkMemoryUtils.WriteFloat(payload, 40, this.imu1GyroX);
        DataLinkMemoryUtils.WriteFloat(payload, 44, this.imu1GyroY);
        DataLinkMemoryUtils.WriteFloat(payload, 48, this.imu1GyroZ);
        DataLinkMemoryUtils.WriteFloat(payload, 52, this.mag1X);
        DataLinkMemoryUtils.WriteFloat(payload, 56, this.mag1Y);
        DataLinkMemoryUtils.WriteFloat(payload, 60, this.mag1Z);
        DataLinkMemoryUtils.WriteInt32(payload, 64, this.baro1Pressure);
        DataLinkMemoryUtils.WriteFloat(payload, 68, this.baro1Temperature);
        DataLinkMemoryUtils.WriteFloat(payload, 72, this.baro1Height);
        DataLinkMemoryUtils.WriteUint8(payload, 76, this.readFlags);
        DataLinkMemoryUtils.WriteUint8(payload, 77, this.gps1Sats);

        return new datalink_message(MSG_ID, payload);
    }

    public static sitl_request_data Unpack(datalink_message message)
    {
        if (message.msg_id != MSG_ID || message.len != MSG_SIZE)
        {
            return null;
        }

        sitl_request_data frame = new sitl_request_data();

        frame.gps1Lat = DataLinkMemoryUtils.ReadDouble(message.payload, 0);
        frame.gps1Lon = DataLinkMemoryUtils.ReadDouble(message.payload, 8);
        frame.gps1Alt = DataLinkMemoryUtils.ReadDouble(message.payload, 16);
        frame.imu1dt = DataLinkMemoryUtils.ReadFloat(message.payload, 24);
        frame.imu1AccX = DataLinkMemoryUtils.ReadFloat(message.payload, 28);
        frame.imu1AccY = DataLinkMemoryUtils.ReadFloat(message.payload, 32);
        frame.imu1AccZ = DataLinkMemoryUtils.ReadFloat(message.payload, 36);
        frame.imu1GyroX = DataLinkMemoryUtils.ReadFloat(message.payload, 40);
        frame.imu1GyroY = DataLinkMemoryUtils.ReadFloat(message.payload, 44);
        frame.imu1GyroZ = DataLinkMemoryUtils.ReadFloat(message.payload, 48);
        frame.mag1X = DataLinkMemoryUtils.ReadFloat(message.payload, 52);
        frame.mag1Y = DataLinkMemoryUtils.ReadFloat(message.payload, 56);
        frame.mag1Z = DataLinkMemoryUtils.ReadFloat(message.payload, 60);
        frame.baro1Pressure = DataLinkMemoryUtils.ReadInt32(message.payload, 64);
        frame.baro1Temperature = DataLinkMemoryUtils.ReadFloat(message.payload, 68);
        frame.baro1Height = DataLinkMemoryUtils.ReadFloat(message.payload, 72);
        frame.readFlags = DataLinkMemoryUtils.ReadUint8(message.payload, 76);
        frame.gps1Sats = DataLinkMemoryUtils.ReadUint8(message.payload, 77);

        return frame;
    }
}

public class sitl_response_data
{
    public const int MSG_ID = 33;
    public const int MSG_SIZE = 66;

    public float velN { get; set; }
    public float velE { get; set; }
    public float velD { get; set; }
    public float posN { get; set; }
    public float posE { get; set; }
    public float posD { get; set; }
    public float qw { get; set; }
    public float qx { get; set; }
    public float qy { get; set; }
    public float qz { get; set; }
    public float angle_fin1 { get; set; }
    public float angle_fin2 { get; set; }
    public float angle_fin3 { get; set; }
    public float angle_fin4 { get; set; }
    public float angle_airbrake { get; set; }
    public float predictedApogee { get; set; }
    public byte smState { get; set; } // Based on state_machine_state
    public byte ignFiredFlags { get; set; } // Each bit represents whether a specific igniter is fired (1) or not (0)

    public datalink_message Pack()
    {
        byte[] payload = new byte[MSG_SIZE];

        DataLinkMemoryUtils.WriteFloat(payload, 0, this.velN);
        DataLinkMemoryUtils.WriteFloat(payload, 4, this.velE);
        DataLinkMemoryUtils.WriteFloat(payload, 8, this.velD);
        DataLinkMemoryUtils.WriteFloat(payload, 12, this.posN);
        DataLinkMemoryUtils.WriteFloat(payload, 16, this.posE);
        DataLinkMemoryUtils.WriteFloat(payload, 20, this.posD);
        DataLinkMemoryUtils.WriteFloat(payload, 24, this.qw);
        DataLinkMemoryUtils.WriteFloat(payload, 28, this.qx);
        DataLinkMemoryUtils.WriteFloat(payload, 32, this.qy);
        DataLinkMemoryUtils.WriteFloat(payload, 36, this.qz);
        DataLinkMemoryUtils.WriteFloat(payload, 40, this.angle_fin1);
        DataLinkMemoryUtils.WriteFloat(payload, 44, this.angle_fin2);
        DataLinkMemoryUtils.WriteFloat(payload, 48, this.angle_fin3);
        DataLinkMemoryUtils.WriteFloat(payload, 52, this.angle_fin4);
        DataLinkMemoryUtils.WriteFloat(payload, 56, this.angle_airbrake);
        DataLinkMemoryUtils.WriteFloat(payload, 60, this.predictedApogee);
        DataLinkMemoryUtils.WriteUint8(payload, 64, this.smState);
        DataLinkMemoryUtils.WriteUint8(payload, 65, this.ignFiredFlags);

        return new datalink_message(MSG_ID, payload);
    }

    public static sitl_response_data Unpack(datalink_message message)
    {
        if (message.msg_id != MSG_ID || message.len != MSG_SIZE)
        {
            return null;
        }

        sitl_response_data frame = new sitl_response_data();

        frame.velN = DataLinkMemoryUtils.ReadFloat(message.payload, 0);
        frame.velE = DataLinkMemoryUtils.ReadFloat(message.payload, 4);
        frame.velD = DataLinkMemoryUtils.ReadFloat(message.payload, 8);
        frame.posN = DataLinkMemoryUtils.ReadFloat(message.payload, 12);
        frame.posE = DataLinkMemoryUtils.ReadFloat(message.payload, 16);
        frame.posD = DataLinkMemoryUtils.ReadFloat(message.payload, 20);
        frame.qw = DataLinkMemoryUtils.ReadFloat(message.payload, 24);
        frame.qx = DataLinkMemoryUtils.ReadFloat(message.payload, 28);
        frame.qy = DataLinkMemoryUtils.ReadFloat(message.payload, 32);
        frame.qz = DataLinkMemoryUtils.ReadFloat(message.payload, 36);
        frame.angle_fin1 = DataLinkMemoryUtils.ReadFloat(message.payload, 40);
        frame.angle_fin2 = DataLinkMemoryUtils.ReadFloat(message.payload, 44);
        frame.angle_fin3 = DataLinkMemoryUtils.ReadFloat(message.payload, 48);
        frame.angle_fin4 = DataLinkMemoryUtils.ReadFloat(message.payload, 52);
        frame.angle_airbrake = DataLinkMemoryUtils.ReadFloat(message.payload, 56);
        frame.predictedApogee = DataLinkMemoryUtils.ReadFloat(message.payload, 60);
        frame.smState = DataLinkMemoryUtils.ReadUint8(message.payload, 64);
        frame.ignFiredFlags = DataLinkMemoryUtils.ReadUint8(message.payload, 65);

        return frame;
    }
}

public class telemetry_data_obc
{
    public const int MSG_ID = 0;
    public const int MSG_SIZE = 29;

    public int lat { get; set; }
    public int lon { get; set; }
    public short qw { get; set; }
    public short qx { get; set; }
    public short qy { get; set; }
    public short qz { get; set; }
    public ushort velocity_kmh { get; set; }
    public ushort batteryVoltage100 { get; set; }
    public ushort alt { get; set; }
    public byte batteryPercentage { get; set; }
    public byte gpsData { get; set; } // First bit - GPS fix, rest - sats number
    public byte state { get; set; }
    public byte sendResponse { get; set; }
    public byte stateFlags { get; set; } // Based on telemetry_data_state_flags
    public byte cmd_seq { get; set; }
    public byte cmd_last_status { get; set; } // Based on telemetry_cmd_status

    public datalink_message Pack()
    {
        byte[] payload = new byte[MSG_SIZE];

        DataLinkMemoryUtils.WriteInt32(payload, 0, this.lat);
        DataLinkMemoryUtils.WriteInt32(payload, 4, this.lon);
        DataLinkMemoryUtils.WriteInt16(payload, 8, this.qw);
        DataLinkMemoryUtils.WriteInt16(payload, 10, this.qx);
        DataLinkMemoryUtils.WriteInt16(payload, 12, this.qy);
        DataLinkMemoryUtils.WriteInt16(payload, 14, this.qz);
        DataLinkMemoryUtils.WriteUint16(payload, 16, this.velocity_kmh);
        DataLinkMemoryUtils.WriteUint16(payload, 18, this.batteryVoltage100);
        DataLinkMemoryUtils.WriteUint16(payload, 20, this.alt);
        DataLinkMemoryUtils.WriteUint8(payload, 22, this.batteryPercentage);
        DataLinkMemoryUtils.WriteUint8(payload, 23, this.gpsData);
        DataLinkMemoryUtils.WriteUint8(payload, 24, this.state);
        DataLinkMemoryUtils.WriteUint8(payload, 25, this.sendResponse);
        DataLinkMemoryUtils.WriteUint8(payload, 26, this.stateFlags);
        DataLinkMemoryUtils.WriteUint8(payload, 27, this.cmd_seq);
        DataLinkMemoryUtils.WriteUint8(payload, 28, this.cmd_last_status);

        return new datalink_message(MSG_ID, payload);
    }

    public static telemetry_data_obc Unpack(datalink_message message)
    {
        if (message.msg_id != MSG_ID || message.len != MSG_SIZE)
        {
            return null;
        }

        telemetry_data_obc frame = new telemetry_data_obc();

        frame.lat = DataLinkMemoryUtils.ReadInt32(message.payload, 0);
        frame.lon = DataLinkMemoryUtils.ReadInt32(message.payload, 4);
        frame.qw = DataLinkMemoryUtils.ReadInt16(message.payload, 8);
        frame.qx = DataLinkMemoryUtils.ReadInt16(message.payload, 10);
        frame.qy = DataLinkMemoryUtils.ReadInt16(message.payload, 12);
        frame.qz = DataLinkMemoryUtils.ReadInt16(message.payload, 14);
        frame.velocity_kmh = DataLinkMemoryUtils.ReadUint16(message.payload, 16);
        frame.batteryVoltage100 = DataLinkMemoryUtils.ReadUint16(message.payload, 18);
        frame.alt = DataLinkMemoryUtils.ReadUint16(message.payload, 20);
        frame.batteryPercentage = DataLinkMemoryUtils.ReadUint8(message.payload, 22);
        frame.gpsData = DataLinkMemoryUtils.ReadUint8(message.payload, 23);
        frame.state = DataLinkMemoryUtils.ReadUint8(message.payload, 24);
        frame.sendResponse = DataLinkMemoryUtils.ReadUint8(message.payload, 25);
        frame.stateFlags = DataLinkMemoryUtils.ReadUint8(message.payload, 26);
        frame.cmd_seq = DataLinkMemoryUtils.ReadUint8(message.payload, 27);
        frame.cmd_last_status = DataLinkMemoryUtils.ReadUint8(message.payload, 28);

        return frame;
    }
}

public class telemetry_data_gcs
{
    public const int MSG_ID = 1;
    public const int MSG_SIZE = 40;

    public int lat { get; set; }
    public int lon { get; set; }
    public int gcsLat { get; set; }
    public int gcsLon { get; set; }
    public short qw { get; set; }
    public short qx { get; set; }
    public short qy { get; set; }
    public short qz { get; set; }
    public ushort velocity_kmh { get; set; }
    public ushort batteryVoltage100 { get; set; }
    public ushort alt { get; set; }
    public ushort packetsReceived { get; set; }
    public ushort packetsTransmitted { get; set; }
    public byte batteryPercentage { get; set; }
    public byte gpsData { get; set; } // First bit - GPS fix, rest - sats number
    public byte state { get; set; }
    public byte stateFlags { get; set; } // Based on telemetry_data_state_flags
    public byte signalStrengthNeg { get; set; }
    public byte packetLossPercentage { get; set; }

    public datalink_message Pack()
    {
        byte[] payload = new byte[MSG_SIZE];

        DataLinkMemoryUtils.WriteInt32(payload, 0, this.lat);
        DataLinkMemoryUtils.WriteInt32(payload, 4, this.lon);
        DataLinkMemoryUtils.WriteInt32(payload, 8, this.gcsLat);
        DataLinkMemoryUtils.WriteInt32(payload, 12, this.gcsLon);
        DataLinkMemoryUtils.WriteInt16(payload, 16, this.qw);
        DataLinkMemoryUtils.WriteInt16(payload, 18, this.qx);
        DataLinkMemoryUtils.WriteInt16(payload, 20, this.qy);
        DataLinkMemoryUtils.WriteInt16(payload, 22, this.qz);
        DataLinkMemoryUtils.WriteUint16(payload, 24, this.velocity_kmh);
        DataLinkMemoryUtils.WriteUint16(payload, 26, this.batteryVoltage100);
        DataLinkMemoryUtils.WriteUint16(payload, 28, this.alt);
        DataLinkMemoryUtils.WriteUint16(payload, 30, this.packetsReceived);
        DataLinkMemoryUtils.WriteUint16(payload, 32, this.packetsTransmitted);
        DataLinkMemoryUtils.WriteUint8(payload, 34, this.batteryPercentage);
        DataLinkMemoryUtils.WriteUint8(payload, 35, this.gpsData);
        DataLinkMemoryUtils.WriteUint8(payload, 36, this.state);
        DataLinkMemoryUtils.WriteUint8(payload, 37, this.stateFlags);
        DataLinkMemoryUtils.WriteUint8(payload, 38, this.signalStrengthNeg);
        DataLinkMemoryUtils.WriteUint8(payload, 39, this.packetLossPercentage);

        return new datalink_message(MSG_ID, payload);
    }

    public static telemetry_data_gcs Unpack(datalink_message message)
    {
        if (message.msg_id != MSG_ID || message.len != MSG_SIZE)
        {
            return null;
        }

        telemetry_data_gcs frame = new telemetry_data_gcs();

        frame.lat = DataLinkMemoryUtils.ReadInt32(message.payload, 0);
        frame.lon = DataLinkMemoryUtils.ReadInt32(message.payload, 4);
        frame.gcsLat = DataLinkMemoryUtils.ReadInt32(message.payload, 8);
        frame.gcsLon = DataLinkMemoryUtils.ReadInt32(message.payload, 12);
        frame.qw = DataLinkMemoryUtils.ReadInt16(message.payload, 16);
        frame.qx = DataLinkMemoryUtils.ReadInt16(message.payload, 18);
        frame.qy = DataLinkMemoryUtils.ReadInt16(message.payload, 20);
        frame.qz = DataLinkMemoryUtils.ReadInt16(message.payload, 22);
        frame.velocity_kmh = DataLinkMemoryUtils.ReadUint16(message.payload, 24);
        frame.batteryVoltage100 = DataLinkMemoryUtils.ReadUint16(message.payload, 26);
        frame.alt = DataLinkMemoryUtils.ReadUint16(message.payload, 28);
        frame.packetsReceived = DataLinkMemoryUtils.ReadUint16(message.payload, 30);
        frame.packetsTransmitted = DataLinkMemoryUtils.ReadUint16(message.payload, 32);
        frame.batteryPercentage = DataLinkMemoryUtils.ReadUint8(message.payload, 34);
        frame.gpsData = DataLinkMemoryUtils.ReadUint8(message.payload, 35);
        frame.state = DataLinkMemoryUtils.ReadUint8(message.payload, 36);
        frame.stateFlags = DataLinkMemoryUtils.ReadUint8(message.payload, 37);
        frame.signalStrengthNeg = DataLinkMemoryUtils.ReadUint8(message.payload, 38);
        frame.packetLossPercentage = DataLinkMemoryUtils.ReadUint8(message.payload, 39);

        return frame;
    }
}

public class telemetry_response
{
    public const int MSG_ID = 2;
    public const int MSG_SIZE = 2;

    public byte cmd { get; set; } // Based on telemetry_cmd
    public byte cmd_seq { get; set; }

    public datalink_message Pack()
    {
        byte[] payload = new byte[MSG_SIZE];

        DataLinkMemoryUtils.WriteUint8(payload, 0, this.cmd);
        DataLinkMemoryUtils.WriteUint8(payload, 1, this.cmd_seq);

        return new datalink_message(MSG_ID, payload);
    }

    public static telemetry_response Unpack(datalink_message message)
    {
        if (message.msg_id != MSG_ID || message.len != MSG_SIZE)
        {
            return null;
        }

        telemetry_response frame = new telemetry_response();

        frame.cmd = DataLinkMemoryUtils.ReadUint8(message.payload, 0);
        frame.cmd_seq = DataLinkMemoryUtils.ReadUint8(message.payload, 1);

        return frame;
    }
}

public class telemetry_gcs_cmd
{
    public const int MSG_ID = 3;
    public const int MSG_SIZE = 1;

    public byte cmd { get; set; } // Based on telemetry_cmd

    public datalink_message Pack()
    {
        byte[] payload = new byte[MSG_SIZE];

        DataLinkMemoryUtils.WriteUint8(payload, 0, this.cmd);

        return new datalink_message(MSG_ID, payload);
    }

    public static telemetry_gcs_cmd Unpack(datalink_message message)
    {
        if (message.msg_id != MSG_ID || message.len != MSG_SIZE)
        {
            return null;
        }

        telemetry_gcs_cmd frame = new telemetry_gcs_cmd();

        frame.cmd = DataLinkMemoryUtils.ReadUint8(message.payload, 0);

        return frame;
    }
}

public class gcs_ack
{
    public const int MSG_ID = 4;
    public const int MSG_SIZE = 1;

    public byte success { get; set; }

    public datalink_message Pack()
    {
        byte[] payload = new byte[MSG_SIZE];

        DataLinkMemoryUtils.WriteUint8(payload, 0, this.success);

        return new datalink_message(MSG_ID, payload);
    }

    public static gcs_ack Unpack(datalink_message message)
    {
        if (message.msg_id != MSG_ID || message.len != MSG_SIZE)
        {
            return null;
        }

        gcs_ack frame = new gcs_ack();

        frame.success = DataLinkMemoryUtils.ReadUint8(message.payload, 0);

        return frame;
    }
}

public class gcs_nack
{
    public const int MSG_ID = 5;
    public const int MSG_SIZE = 0;


    public datalink_message Pack()
    {
        return new datalink_message(MSG_ID, null);
    }

    public static gcs_nack Unpack(datalink_message message)
    {
        if (message.msg_id != MSG_ID || message.len != MSG_SIZE)
        {
            return null;
        }

        gcs_nack frame = new gcs_nack();


        return frame;
    }
}

public class radio_module_tx_done
{
    public const int MSG_ID = 6;
    public const int MSG_SIZE = 0;


    public datalink_message Pack()
    {
        return new datalink_message(MSG_ID, null);
    }

    public static radio_module_tx_done Unpack(datalink_message message)
    {
        if (message.msg_id != MSG_ID || message.len != MSG_SIZE)
        {
            return null;
        }

        radio_module_tx_done frame = new radio_module_tx_done();


        return frame;
    }
}



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