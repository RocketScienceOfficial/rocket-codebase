using DataLink;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class OrientationController : MonoBehaviour
{
    private void Start()
    {
        CommunicationManager.Instance.OnConnected += (sender, args) =>
        {
            transform.localRotation = new Quaternion(0f, 0f, 0f, 1f);
        };

        CommunicationManager.Instance.OnRead += (sender, args) =>
        {
            var msg = args.Message;

            if (msg.msg_id == telemetry_data_gcs.MSG_ID)
            {
                var payload = telemetry_data_gcs.Unpack(msg);

                var qw = (float)payload.qw / short.MaxValue;
                var qx = (float)payload.qx / short.MaxValue;
                var qy = (float)payload.qy / short.MaxValue;
                var qz = (float)payload.qz / short.MaxValue;
                
                transform.localRotation = new Quaternion(payload.qx, payload.qy, payload.qz, payload.qw);
            }
        };
    }
}