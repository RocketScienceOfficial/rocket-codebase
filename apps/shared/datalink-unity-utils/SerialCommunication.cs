using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Ports;
using System.Threading;
using UnityEngine;

namespace DataLink
{
    public class SerialCommunication : CommunicationProtocolBase
    {
        private SerialPort _currentSerialPort;
        private Thread _readThread;

        public override void Connect(string destination)
        {
            new Thread(() =>
            {
                try
                {
                    if (_currentSerialPort != null)
                    {
                        Debug.Log("Serial port is already connected (or trying to connect)");

                        return;
                    }

                    Debug.Log("Connecting to port: " + destination + "...");

                    _currentSerialPort = new SerialPort()
                    {
                        PortName = destination,
                        BaudRate = 115200,
                        Parity = Parity.None,
                        DataBits = 8,
                        StopBits = StopBits.One,
                        RtsEnable = true,
                        DtrEnable = true,
                        ReadTimeout = 1000,
                        WriteTimeout = 1000,
                    };

                    _currentSerialPort.Open();

                    _onConnected?.Invoke();

                    _readThread = new Thread(ReadThread);
                    _readThread.Start();

                    Debug.Log("COM Port Connected!");
                }
                catch (Exception ex)
                {
                    Debug.Log("Could not find serial port: " + ex);

                    Disconnect();
                }
            }).Start();
        }

        public override void Disconnect()
        {
            new Thread(() =>
            {
                if (_currentSerialPort == null)
                {
                    Debug.Log("Couldn't disconnect since serial port isn't connected!");

                    return;
                }

                Debug.Log("Begining disconnecting...");

                _currentSerialPort?.Close();
                _currentSerialPort = null;

                Debug.Log("COM Port Disconnected!");

                if (_readThread != null)
                {
                    Debug.Log("Closing read thread...");

                    _readThread.Join();
                    _readThread = null;

                    Debug.Log("Read thread closed!");
                }

                _onDisconnected?.Invoke();
            }).Start();
        }

        public override bool IsConnected()
        {
            return _currentSerialPort != null && _currentSerialPort.IsOpen;
        }

        private void ReadThread()
        {
            const int BUFFER_SIZE = 512;

            var buffer = new List<byte>();

            while (IsConnected())
            {
                try
                {
                    var b = (byte)_currentSerialPort.ReadByte();

                    if (buffer.Count >= BUFFER_SIZE)
                    {
                        Debug.Log("Buffer overflow! Clearing buffer...");

                        buffer.Clear();
                    }

                    buffer.Add(b);

                    if (b == 0x00)
                    {
                        Debug.Log("Received " + buffer.Count + " bytes from serial");

                        var message = DataLinkSerial.Deserialize(buffer.ToArray());

                        if (message != null)
                        {
                            _onMessageReceived?.Invoke(message);
                        }
                        else
                        {
                            Debug.Log("Couldn't deserialize message!");
                        }

                        buffer.Clear();
                    }
                }
                catch (TimeoutException) { }
                catch (InvalidOperationException) { }
                catch (IOException)
                {
                    Debug.Log("There was an error during reading serial port. Aborting...");

                    Disconnect();

                    break;
                }
            }
        }

        public override void SendData(datalink_message message)
        {
            if (_currentSerialPort != null)
            {
                var bytes = DataLinkSerial.Serialize(message);

                if (bytes == null)
                {
                    Debug.Log("Error while serialized message!");

                    return;
                }

                _currentSerialPort.Write(bytes, 0, bytes.Length);

                Debug.Log("Written to serial port " + bytes.Length + " bytes!");
            }
        }
    }
}