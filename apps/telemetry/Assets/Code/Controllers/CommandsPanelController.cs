using DataLink;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using TMPro;
using UnityEngine;

public class CommandsPanelController : MonoBehaviour
{
    [SerializeField] private Transform m_CommandsContainer;
    [SerializeField] private GameObject m_CommandPrefab;

    private void Start()
    {
        Commander.Instance.OnCommandsChanged += (sender, args) =>
        {
            UpdateCommandsUI();
        };
    }

    private void UpdateCommandsUI()
    {
        for (var i = 0; i < m_CommandsContainer.childCount; i++)
        {
            Destroy(m_CommandsContainer.GetChild(i).gameObject);
        }

        foreach (var cmd in Commander.Instance.GetAckedCommands().Reverse())
        {
            var cmdObj = Instantiate(m_CommandPrefab, m_CommandsContainer);

            SetupCommandObject(cmdObj, cmd.cmd, cmd.success ? "ACK" : "FAIL", cmd.success ? Color.green : Color.red);
        }

        foreach (var cmdInfo in Commander.Instance.GetPendingCommands())
        {
            var cmdObj = Instantiate(m_CommandPrefab, m_CommandsContainer);

            SetupCommandObject(cmdObj, cmdInfo.Item1, cmdInfo.Item2.ToString(), Color.white);
        }

        foreach (var cmd in Commander.Instance.GetNackedCommands().Reverse())
        {
            var cmdObj = Instantiate(m_CommandPrefab, m_CommandsContainer);

            SetupCommandObject(cmdObj, cmd.cmd, "NACK", Color.red);
        }
    }

    private void SetupCommandObject(GameObject obj, telemetry_cmd cmd, string suffix, Color color)
    {
        var text = obj.GetComponent<TextMeshProUGUI>();

        text.SetText(TranslateCommand(cmd) + " (" + suffix + ")");
        text.color = color;
    }

    private string TranslateCommand(telemetry_cmd cmd)
    {
        return cmd switch
        {
            telemetry_cmd.DATALINK_TELEMETRY_CMD_IGN_1_REQ_FIRE => "IGN_1_REQ_FIRE",
            telemetry_cmd.DATALINK_TELEMETRY_CMD_IGN_2_REQ_FIRE => "IGN_2_REQ_FIRE",
            telemetry_cmd.DATALINK_TELEMETRY_CMD_IGN_3_REQ_FIRE => "IGN_3_REQ_FIRE",
            telemetry_cmd.DATALINK_TELEMETRY_CMD_IGN_4_REQ_FIRE => "IGN_4_REQ_FIRE",
            telemetry_cmd.DATALINK_TELEMETRY_CMD_ARM => "ARM",
            telemetry_cmd.DATALINK_TELEMETRY_CMD_DISARM => "DISARM",
            telemetry_cmd.DATALINK_TELEMETRY_CMD_3V3_ENABLED => "ENABLE_3V3",
            telemetry_cmd.DATALINK_TELEMETRY_CMD_3V3_DISABLED => "DISABLE_3V3",
            telemetry_cmd.DATALINK_TELEMETRY_CMD_5V_ENABLED => "ENABLE_5V",
            telemetry_cmd.DATALINK_TELEMETRY_CMD_5V_DISABLED => "DISABLE_5V",
            telemetry_cmd.DATALINK_TELEMETRY_CMD_VBAT_ENABLED => "ENABLE_VBAT",
            telemetry_cmd.DATALINK_TELEMETRY_CMD_VBAT_DISABLED => "DISABLE_VBAT",
            _ => "UNKNOWN"
        };
    }
}