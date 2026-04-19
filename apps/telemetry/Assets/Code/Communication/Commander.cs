using DataLink;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;

public class Commander : MonoBehaviour
{
    private const int MAX_RETRIES = 2;

    public static Commander Instance { get; private set; }

    public event EventHandler OnCommandsChanged;

    private readonly Queue<PendingCommand> _queue = new();
    private readonly List<AckedCommand> _ackedCommands = new();
    private readonly List<NackedCommand> _nackedCommands = new();

    private void Awake()
    {
        Instance = this;
    }

    private void Start()
    {
        CommunicationManager.Instance.OnConnected += (sender, args) =>
        {
            print("Serial port connected. Clearing command queue.");

            _queue.Clear();
            _ackedCommands.Clear();
            _nackedCommands.Clear();

            OnCommandsChanged?.Invoke(this, EventArgs.Empty);
        };

        CommunicationManager.Instance.OnRead += (sender, args) =>
        {
            var msg = args.Message;

            if (msg.msg_id == gcs_ack.MSG_ID)
            {
                var ack = gcs_ack.Unpack(msg);
                var c = _queue.Dequeue();
                c.onFinish?.Invoke();

                _ackedCommands.Add(new AckedCommand { cmd = c.cmd, success = ack.success == 1 });

                print($"Command {c.cmd} acknowledged!");

                if (_queue.Count > 0)
                {
                    SendNext();
                }

                OnCommandsChanged?.Invoke(this, EventArgs.Empty);
            }
            else if (msg.msg_id == gcs_nack.MSG_ID)
            {
                if (_queue.Count > 0)
                {
                    var c = _queue.Dequeue();

                    if (c.retriesLeft > 0)
                    {
                        c.retriesLeft--;
                        _queue.Enqueue(c);

                        print($"Command {c.cmd} not acknowledged. Retrying... ({c.retriesLeft} retries left)");
                    }
                    else
                    {
                        c.onFinish?.Invoke();

                        _nackedCommands.Add(new NackedCommand { cmd = c.cmd });

                        print($"Failed to send command {c.cmd} after {MAX_RETRIES} retries.");
                    }
                }

                if (_queue.Count > 0)
                {
                    SendNext();
                }

                OnCommandsChanged?.Invoke(this, EventArgs.Empty);
            }
        };
    }


    private void SendNext()
    {
        var nextCmd = _queue.Peek();

        print("Sending command: " + nextCmd.cmd);

        CommunicationManager.Instance.GetProtocol().SendData(new telemetry_gcs_cmd { cmd = (byte)nextCmd.cmd }.Pack());
    }


    public void AddCommandToSendQueue(telemetry_cmd cmd, Action onFinish = null)
    {
        _queue.Enqueue(new PendingCommand
        {
            cmd = cmd,
            retriesLeft = MAX_RETRIES,
            onFinish = onFinish
        });

        if (_queue.Count == 1)
        {
            SendNext();
        }

        OnCommandsChanged?.Invoke(this, EventArgs.Empty);
    }

    public IEnumerable<AckedCommand> GetAckedCommands() => _ackedCommands;
    public IEnumerable<NackedCommand> GetNackedCommands() => _nackedCommands;
    public IEnumerable<(telemetry_cmd, int)> GetPendingCommands() => _queue.Select(q => (q.cmd, q.retriesLeft));


    public struct PendingCommand
    {
        public telemetry_cmd cmd;
        public int retriesLeft;
        public Action onFinish;
    }

    public struct AckedCommand
    {
        public telemetry_cmd cmd;
        public bool success;
    }

    public struct NackedCommand
    {
        public telemetry_cmd cmd;
    }
}