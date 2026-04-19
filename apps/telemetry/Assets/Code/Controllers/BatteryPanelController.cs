using DataLink;
using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class BatteryPanelController : MonoBehaviour
{
    [SerializeField] private TextMeshProUGUI m_PercentageText;
    [SerializeField] private TextMeshProUGUI m_VoltageText;
    [SerializeField] private Image m_Fill;

    private void Start()
    {
        CommunicationManager.Instance.OnConnected += (sender, args) =>
        {
            SetData(0, 0);
        };

        CommunicationManager.Instance.OnRead += (sender, args) =>
        {
            var msg = args.Message;

            if (msg.msg_id == telemetry_data_gcs.MSG_ID)
            {
                var payload = telemetry_data_gcs.Unpack(msg);

                SetData(payload.batteryPercentage, payload.batteryVoltage100 / 100f);
            }
        };
    }

    private void SetData(int batteryPercentage, float batteryVoltage)
    {
        m_PercentageText.SetText(batteryPercentage + "%");
        m_VoltageText.SetText(string.Format("{0:0.00}", batteryVoltage).Replace(',', '.') + "V");
        m_Fill.fillAmount = 1f - batteryPercentage / 100f;
    }
}