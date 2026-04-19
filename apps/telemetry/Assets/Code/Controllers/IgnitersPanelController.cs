using DataLink;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class IgnitersPanelController : MonoBehaviour
{
    [SerializeField] private Image m_IGN1StatusImage;
    [SerializeField] private Image m_IGN2StatusImage;
    [SerializeField] private Image m_IGN3StatusImage;
    [SerializeField] private Image m_IGN4StatusImage;

    [SerializeField] private GameObject m_ConfirmFirePanel;
    [SerializeField] private Button m_ConfirmFireButton;
    [SerializeField] private Button m_IGN1FireButton;
    [SerializeField] private Button m_IGN2FireButton;
    [SerializeField] private Button m_IGN3FireButton;
    [SerializeField] private Button m_IGN4FireButton;

    private telemetry_cmd _currentCMD;

    private void Start()
    {
        SetupIGNFireButton(m_IGN1FireButton, telemetry_cmd.DATALINK_TELEMETRY_CMD_IGN_1_REQ_FIRE);
        SetupIGNFireButton(m_IGN2FireButton, telemetry_cmd.DATALINK_TELEMETRY_CMD_IGN_2_REQ_FIRE);
        SetupIGNFireButton(m_IGN3FireButton, telemetry_cmd.DATALINK_TELEMETRY_CMD_IGN_3_REQ_FIRE);
        SetupIGNFireButton(m_IGN4FireButton, telemetry_cmd.DATALINK_TELEMETRY_CMD_IGN_4_REQ_FIRE);

        m_ConfirmFireButton.onClick.AddListener(() =>
        {
            m_ConfirmFirePanel.SetActive(false);

            Commander.Instance.AddCommandToSendQueue(_currentCMD);
        });

        CommunicationManager.Instance.OnConnected += (sender, args) =>
        {
            SetIGNStatusImageUI(m_IGN1StatusImage, false);
            SetIGNStatusImageUI(m_IGN2StatusImage, false);
            SetIGNStatusImageUI(m_IGN3StatusImage, false);
            SetIGNStatusImageUI(m_IGN4StatusImage, false);
        };

        CommunicationManager.Instance.OnRead += (sender, args) =>
        {
            var msg = args.Message;

            if (msg.msg_id == telemetry_data_gcs.MSG_ID)
            {
                var payload = telemetry_data_gcs.Unpack(msg);

                SetIGNStatusImageUI(m_IGN1StatusImage, (payload.stateFlags & (byte)telemetry_data_state_flags.DATALINK_FLAGS_TELEMETRY_DATA_CONTROL_IGN_1_CONT) > 0);
                SetIGNStatusImageUI(m_IGN2StatusImage, (payload.stateFlags & (byte)telemetry_data_state_flags.DATALINK_FLAGS_TELEMETRY_DATA_CONTROL_IGN_2_CONT) > 0);
                SetIGNStatusImageUI(m_IGN3StatusImage, (payload.stateFlags & (byte)telemetry_data_state_flags.DATALINK_FLAGS_TELEMETRY_DATA_CONTROL_IGN_3_CONT) > 0);
                SetIGNStatusImageUI(m_IGN4StatusImage, (payload.stateFlags & (byte)telemetry_data_state_flags.DATALINK_FLAGS_TELEMETRY_DATA_CONTROL_IGN_4_CONT) > 0);
            }
        };
    }

    private void SetupIGNFireButton(Button btn, telemetry_cmd cmd)
    {
        btn.onClick.AddListener(() =>
        {
            _currentCMD = cmd;

            m_ConfirmFirePanel.SetActive(true);
        });
    }

    private void SetIGNStatusImageUI(Image img, bool cont)
    {
        img.color = cont ? Color.green : Color.red;
    }
}