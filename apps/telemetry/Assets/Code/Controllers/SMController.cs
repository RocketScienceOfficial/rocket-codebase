using DataLink;
using TMPro;
using UnityEngine;

public class SMController : MonoBehaviour
{
    private static readonly string[] STATES_NAMES = new string[]
    {
        "Standing",
        "Armed",
        "Accelerating",
        "Free Flight",
        "Free Fall",
        "Landed",
    };

    [SerializeField] private TextMeshProUGUI m_StatusText;

    private void Start()
    {
        CommunicationManager.Instance.OnConnected += (sender, args) =>
        {
            UpdateCheckpointText(0);
        };

        CommunicationManager.Instance.OnRead += (sender, args) =>
        {
            var msg = args.Message;

            if (msg.msg_id == telemetry_data_gcs.MSG_ID)
            {
                var payload = telemetry_data_gcs.Unpack(msg);

                UpdateCheckpointText(payload.state);
            }
        };
    }

    private void UpdateCheckpointText(int state)
    {
        var text = "Unknown";

        if (state >= 0 && state < STATES_NAMES.Length)
        {
            text = STATES_NAMES[state];
        }

        m_StatusText.SetText(text.ToUpper());
    }
}