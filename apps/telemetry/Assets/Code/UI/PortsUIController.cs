using DataLink;
using System.Collections;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class PortsUIController : MonoBehaviour
{
    private const string TCP_DESTINATION = "127.0.0.1:12349";

    [SerializeField] private GameObject m_LoadingPanel;
    [SerializeField] private GameObject m_PortSelectPanel;
    [SerializeField] private Transform m_Parent;
    [SerializeField] private Button m_RefreshButton;

    private void Start()
    {
        CommunicationManager.Instance.OnConnected += (sender, args) =>
        {
            FetchPorts();
        };

        CommunicationManager.Instance.OnDisconnected += (sender, args) =>
        {
            FetchPorts();
        };

        m_RefreshButton.onClick.AddListener(() => FetchPorts());

        FetchPorts();
    }

    private void FetchPorts()
    {
        m_LoadingPanel.SetActive(true);

        Clear();

        if (CommunicationManager.Instance.IsSerial())
        {
            foreach (var port in SerialPort.GetPortNames().OrderBy(p => p))
            {
                SetupMicrocontroller(port);
            }
        }
        else
        {
            SetupMicrocontroller(TCP_DESTINATION);
        }

        m_LoadingPanel.SetActive(false);
    }

    private void Clear()
    {
        for (int i = 0; i < m_Parent.childCount; i++)
        {
            Destroy(m_Parent.GetChild(i).gameObject);
        }
    }

    private void SetupMicrocontroller(string path)
    {
        var obj = Instantiate(m_PortSelectPanel, m_Parent);

        obj.GetComponentInChildren<TextMeshProUGUI>().SetText(path);
        obj.GetComponent<Button>().onClick.AddListener(() =>
        {
            m_LoadingPanel.SetActive(true);

            CommunicationManager.Instance.GetProtocol().Connect(path);
        });
    }
}