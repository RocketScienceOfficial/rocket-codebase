using DataLink;
using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class ControlPanelController : MonoBehaviour
{
    [SerializeField] private Button m_ArmButton;

    [SerializeField] private Button m_3V3Button;
    [SerializeField] private Button m_5VButton;
    [SerializeField] private Button m_VBATButton;

    [SerializeField] private Image m_3V3StatusImage;
    [SerializeField] private Image m_5VStatusImage;
    [SerializeField] private Image m_VBATStatusImage;

    private void Start()
    {
        CommunicationManager.Instance.OnConnected += (sender, args) =>
        {
            SetState(false, false, false, false);
        };

        CommunicationManager.Instance.OnRead += (sender, args) =>
        {
            var msg = args.Message;

            if (msg.msg_id == telemetry_data_gcs.MSG_ID)
            {
                var payload = telemetry_data_gcs.Unpack(msg);

                SetState(payload.state >= (byte)state_machine_state.DATALINK_SM_STATE_ARMED,
                        (payload.stateFlags & (byte)telemetry_data_state_flags.DATALINK_FLAGS_TELEMETRY_DATA_CONTROL_3V3_ENABLED) > 0,
                        (payload.stateFlags & (byte)telemetry_data_state_flags.DATALINK_FLAGS_TELEMETRY_DATA_CONTROL_5V_ENABLED) > 0,
                        (payload.stateFlags & (byte)telemetry_data_state_flags.DATALINK_FLAGS_TELEMETRY_DATA_CONTROL_VBAT_ENABLED) > 0);
            }
        };
    }

    private void SetState(bool armed, bool _3v3, bool _5v, bool vbat)
    {
        m_ArmButton.GetComponentInChildren<TextMeshProUGUI>().SetText(!armed ? "ARM" : "DISARM");
        m_3V3Button.GetComponentInChildren<TextMeshProUGUI>().SetText(!_3v3 ? "ENABLE" : "DISABLE");
        m_5VButton.GetComponentInChildren<TextMeshProUGUI>().SetText(!_5v ? "ENABLE" : "DISABLE");
        m_VBATButton.GetComponentInChildren<TextMeshProUGUI>().SetText(!vbat ? "ENABLE" : "DISABLE");

        m_3V3StatusImage.color = _3v3 ? Color.green : Color.red;
        m_5VStatusImage.color = _5v ? Color.green : Color.red;
        m_VBATStatusImage.color = vbat ? Color.green : Color.red;

        m_ArmButton.onClick.RemoveAllListeners();
        m_ArmButton.onClick.AddListener(() =>
        {
            Commander.Instance.AddCommandToSendQueue(!armed ? telemetry_cmd.DATALINK_TELEMETRY_CMD_ARM : telemetry_cmd.DATALINK_TELEMETRY_CMD_DISARM);
        });

        m_3V3Button.onClick.RemoveAllListeners();
        m_3V3Button.onClick.AddListener(() =>
        {
            Commander.Instance.AddCommandToSendQueue(!_3v3 ? telemetry_cmd.DATALINK_TELEMETRY_CMD_3V3_ENABLED : telemetry_cmd.DATALINK_TELEMETRY_CMD_3V3_DISABLED);
        });

        m_5VButton.onClick.RemoveAllListeners();
        m_5VButton.onClick.AddListener(() =>
        {
            Commander.Instance.AddCommandToSendQueue(!_5v ? telemetry_cmd.DATALINK_TELEMETRY_CMD_5V_ENABLED : telemetry_cmd.DATALINK_TELEMETRY_CMD_5V_DISABLED);
        });

        m_VBATButton.onClick.RemoveAllListeners();
        m_VBATButton.onClick.AddListener(() =>
        {
            Commander.Instance.AddCommandToSendQueue(!vbat ? telemetry_cmd.DATALINK_TELEMETRY_CMD_VBAT_ENABLED : telemetry_cmd.DATALINK_TELEMETRY_CMD_VBAT_DISABLED);
        });
    }
}