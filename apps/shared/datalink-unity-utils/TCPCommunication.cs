using System;
using System.Collections;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Threading;
using UnityEngine;

namespace DataLink
{
    public class TCPCommunication : CommunicationProtocolBase
    {
        private TcpClient _client;
        private NetworkStream _stream;
        private Thread _readThread;

        public override void Connect(string destination)
        {
            try
            {
                if (_client != null)
                {
                    Debug.Log("TCP client is already connected (or trying to connect)");

                    return;
                }

                var host = destination.Split(':')[0];
                var port = int.Parse(destination.Split(':')[1]);

                Debug.Log("Connecting to TCP server at " + host + ":" + port);

                _client = new TcpClient();
                _client.BeginConnect(host, port, (ar) =>
                {
                    _stream = _client.GetStream();

                    _readThread = new Thread(ReadThread);
                    _readThread.Start();

                    if (IsConnected())
                    {
                        Debug.Log("Connected to server");

                        _onConnected?.Invoke();
                    }
                    else
                    {
                        Debug.Log("Failed to connect to server");

                        Disconnect();
                    }
                }, null);
            }
            catch (Exception ex)
            {
                Debug.Log("Error occured while connecting to TCP: " + ex);

                Disconnect();
            }
        }

        public override void Disconnect()
        {
            new Thread(() =>
            {
                if (_client == null || _stream == null)
                {
                    Debug.Log("Couldn't disconnect since TCP isn't connected!");

                    return;
                }

                Debug.Log("Begining disconnecting...");

                _stream?.Close();
                _stream = null;

                _client?.Close();
                _client = null;

                Debug.Log("Disconnected from server");

                if (_readThread != null)
                {
                    Debug.Log("Closing read thread...");

                    _readThread.Join();
                    _readThread = null;
                }

                _onDisconnected?.Invoke();
            }).Start();
        }

        public override bool IsConnected()
        {
            return _client != null && _client.Connected;
        }

        private void ReadThread()
        {
            var buffer = new byte[1024];
            var currentLen = 0;

            while (IsConnected())
            {
                try
                {
                    if (_stream.DataAvailable)
                    {
                        var bytesRead = _stream.Read(buffer, currentLen, buffer.Length - currentLen);

                        if (bytesRead <= 0)
                        {
                            Debug.Log("TCP connection closed by remote host!");

                            Disconnect();

                            return;
                        }

                        currentLen += bytesRead;

                        for (var i = 0; i < currentLen; i++)
                        {
                            if (buffer[i] == 0x00)
                            {
                                var len = i + 1;
                                var messageBytes = new byte[len];
                                Array.Copy(buffer, messageBytes, len);

                                var msg = DataLinkSerial.Deserialize(messageBytes);

                                if (msg != null)
                                {
                                    _onMessageReceived?.Invoke(msg);
                                }
                                else
                                {
                                    Debug.Log("Couldn't deserialize message!");
                                }

                                Array.Copy(buffer, len, buffer, 0, currentLen - len);
                                currentLen -= len;
                            }
                        }
                    }
                }
                catch (ObjectDisposedException)
                {
                    Debug.Log("Cannot read. TCP stream was closed!");

                    Disconnect();

                    return;
                }
            }
        }

        public override void SendData(datalink_message msg)
        {
            if (IsConnected())
            {
                var bytes = DataLinkSerial.Serialize(msg);

                if (bytes == null)
                {
                    Debug.Log("Error while serializing message!");

                    return;
                }

                _stream.Write(bytes, 0, bytes.Length);

                Debug.Log("Written to TCP " + bytes.Length + " bytes!");
            }
        }
    }
}