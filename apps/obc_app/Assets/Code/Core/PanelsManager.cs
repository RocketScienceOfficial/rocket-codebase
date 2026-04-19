using DataLink;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public enum PanelType
{
    Connect,
    Main,
    Download,
    Clear,
    Recovery,
    Igniters,
}

public class PanelsManager : MonoBehaviour
{
    public static PanelsManager Instance { get; private set; }

    [SerializeField] private GameObject m_ConnectPanel;
    [SerializeField] private GameObject m_MainPanel;
    [SerializeField] private GameObject m_DownloadPanel;
    [SerializeField] private GameObject m_ClearPanel;
    [SerializeField] private GameObject m_RecoveryPanel;
    [SerializeField] private GameObject m_IgnitersPanel;

    private void Awake()
    {
        Instance = this;

        DeactiveAllPanels();
        SetPanelActive(PanelType.Connect, true);
    }

    private void Start()
    {
        CommunicationManager.Instance.OnConnected += (sender, args) =>
        {
            DeactiveAllPanels();
            SetPanelActive(PanelType.Main, true);
        };

        CommunicationManager.Instance.OnDisconnected += (sender, args) =>
        {
            DeactiveAllPanels();
            SetPanelActive(PanelType.Connect, true);
        };
    }

    public void DeactiveAllPanels()
    {
        SetPanelActive(PanelType.Connect, false);
        SetPanelActive(PanelType.Main, false);
        SetPanelActive(PanelType.Download, false);
        SetPanelActive(PanelType.Clear, false);
        SetPanelActive(PanelType.Recovery, false);
        SetPanelActive(PanelType.Igniters, false);
    }

    public void SetPanelActive(PanelType panel, bool active)
    {
        switch (panel)
        {
            case PanelType.Connect:
                m_ConnectPanel.SetActive(active);
                break;
            case PanelType.Main:
                m_MainPanel.SetActive(active);
                break;
            case PanelType.Download:
                m_DownloadPanel.SetActive(active);
                break;
            case PanelType.Clear:
                m_ClearPanel.SetActive(active);
                break;
            case PanelType.Recovery:
                m_RecoveryPanel.SetActive(active);
                break;
            case PanelType.Igniters:
                m_IgnitersPanel.SetActive(active);
                break;
            default:
                break;
        }
    }
}