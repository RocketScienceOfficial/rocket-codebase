using DataLink;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class GaugesPanelController : MonoBehaviour
{
    [SerializeField] private GaugePanelController m_SpeedPanel;
    [SerializeField] private GaugePanelController m_AltitudePanel;

    private void Start()
    {
        CommunicationManager.Instance.OnConnected += (sender, args) =>
        {
            SetValues(0, 0);
        };

        CommunicationManager.Instance.OnRead += (sender, args) =>
        {
            var msg = args.Message;

            if (msg.msg_id == telemetry_data_gcs.MSG_ID)
            {
                var payload = telemetry_data_gcs.Unpack(msg);

                SetValues(payload.velocity_kmh, payload.alt);
            }
        };
    }

    private void SetValues(int vel, int alt)
    {
        m_SpeedPanel.SetValue(vel, 0, 2000);
        m_AltitudePanel.SetValue(alt, 0, 3000);
    }
}