using DataLink;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class IgnitersController : MonoBehaviour
{
    [SerializeField] private Button m_OpenPanelButton;
    [SerializeField] private Button m_ExitButton;
    [SerializeField] private Button[] m_IGNButtons;
    [SerializeField] private GameObject m_TestingPanel;

    private bool _isTestingIgn;
    private Watchdog _watchdog;

    private void Start()
    {
        _watchdog = new Watchdog("IgnitersController", 5f, () => SetTestIGN(false));

        m_OpenPanelButton.onClick.AddListener(() =>
        {
            m_TestingPanel.SetActive(false);

            PanelsManager.Instance.SetPanelActive(PanelType.Igniters, true);
        });

        m_ExitButton.onClick.AddListener(() =>
        {
            PanelsManager.Instance.SetPanelActive(PanelType.Igniters, false);
        });

        for (var i = 0; i < m_IGNButtons.Length; i++)
        {
            var num = i + 1;

            m_IGNButtons[i].onClick.AddListener(() =>
            {
                TestIgniter(num);
            });
        }

        CommunicationManager.Instance.OnRead += (sender, args) =>
        {
            if (_isTestingIgn)
            {
                var msg = args.Message;

                if (msg.msg_id == ign_response_test.MSG_ID)
                {
                    SetTestIGN(false);
                }
            }
        };

        CommunicationManager.Instance.OnDisconnected += (sender, args) =>
        {
            if (_isTestingIgn)
            {
                print("Disconnected while testing igniter. Aborting...");

                SetTestIGN(false);

                PanelsManager.Instance.SetPanelActive(PanelType.Igniters, false);
            }
        };

        m_TestingPanel.SetActive(false);
    }

    private void TestIgniter(int num)
    {
        SetTestIGN(true);

        CommunicationManager.Instance.GetProtocol().SendData(new ign_request_test() { ignNum = (byte)num }.Pack());
    }

    private void SetTestIGN(bool en)
    {
        m_TestingPanel.SetActive(en);

        _isTestingIgn = en;

        if (en)
        {
            _watchdog.Enable();
        }
        else
        {
            _watchdog.Disable();
        }
    }
}