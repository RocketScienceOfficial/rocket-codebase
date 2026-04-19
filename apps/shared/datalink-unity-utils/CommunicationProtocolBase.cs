using System;

namespace DataLink
{
    public abstract class CommunicationProtocolBase
    {
        protected Action _onConnected;
        protected Action _onDisconnected;
        protected Action<datalink_message> _onMessageReceived;

        public void Setup(Action onConnected, Action onDisconnected, Action<datalink_message> onMessageReceived)
        {
            _onConnected = onConnected;
            _onDisconnected = onDisconnected;
            _onMessageReceived = onMessageReceived;
        }

        public abstract void Connect(string destination);
        public abstract void Disconnect();
        public abstract bool IsConnected();
        public abstract void SendData(datalink_message message);
    }
}