using System;
using System.Collections.Concurrent;
using UnityEngine;

namespace DataLink
{
    public class CommunicationManager : MonoBehaviour
    {
        public static CommunicationManager Instance { get; private set; }

        public event EventHandler OnConnected;
        public event EventHandler OnDisconnected;
        public event EventHandler<CommunicationManagerOnReadEventArgs> OnRead;

        private readonly ConcurrentQueue<CommunicationManagerThreadAction> _actionsQueue = new();
        private readonly ConcurrentQueue<datalink_message> _readQueue = new();

        [SerializeField] private bool m_UseTCP;

        private CommunicationProtocolBase _com;

        private void Awake()
        {
            Instance = this;

            _com = SpawnProcotol();
            _com.Setup(OnConnectedThread, OnDisconnectedThread, OnReadThread);
        }

        private void Update()
        {
            if (_actionsQueue.TryDequeue(out var action))
            {
                switch (action)
                {
                    case CommunicationManagerThreadAction.Connect:
                        OnConnected?.Invoke(this, EventArgs.Empty);
                        break;
                    case CommunicationManagerThreadAction.Disconnect:
                        OnDisconnected?.Invoke(this, EventArgs.Empty);
                        break;
                }
            }

            if (_readQueue.TryDequeue(out var msg))
            {
                OnRead?.Invoke(this, new CommunicationManagerOnReadEventArgs { Message = msg });
            }
        }

        private void OnApplicationQuit()
        {
            _com.Disconnect();
        }

        private void OnApplicationPause(bool pause)
        {
            if (pause)
            {
                _com.Disconnect();
            }
        }


        private void OnConnectedThread()
        {
            _actionsQueue.Enqueue(CommunicationManagerThreadAction.Connect);
        }

        private void OnDisconnectedThread()
        {
            _actionsQueue.Enqueue(CommunicationManagerThreadAction.Disconnect);
        }

        private void OnReadThread(datalink_message msg)
        {
            _readQueue.Enqueue(msg);
        }


        private CommunicationProtocolBase SpawnProcotol()
        {
#if UNITY_EDITOR
            if (m_UseTCP)
            {
                return new TCPCommunication();
            }
#endif

            return new SerialCommunication();
        }

        public CommunicationProtocolBase GetProtocol()
        {
            return _com;
        }

        public bool IsSerial()
        {
            return !m_UseTCP;
        }
    }

    public enum CommunicationManagerThreadAction
    {
        Connect,
        Disconnect,
    }

    public class CommunicationManagerOnReadEventArgs : EventArgs
    {
        public datalink_message Message { get; set; }
    }
}