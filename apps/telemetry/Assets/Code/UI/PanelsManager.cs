using DataLink;
using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public enum Panel
{
    Visualization,
    PortSelection,
}

public class PanelsManager : MonoBehaviour
{
    [SerializeField] private GameObject m_VisualizationPanel;
    [SerializeField] private GameObject m_PortSelectionPanel;

    private Dictionary<Panel, GameObject> m_Panels;

    private void Awake()
    {
        m_Panels = new()
        {
            { Panel.Visualization, m_VisualizationPanel },
            { Panel.PortSelection, m_PortSelectionPanel },
        };
    }

    private void Start()
    {
        DeactiveAllPanels();
        SetPanelActive(Panel.PortSelection, true);

        CommunicationManager.Instance.OnConnected += (sender, args) =>
        {
            DeactiveAllPanels();
            SetPanelActive(Panel.Visualization, true);
        };

        CommunicationManager.Instance.OnDisconnected += (sender, args) =>
        {
            DeactiveAllPanels();
            SetPanelActive(Panel.PortSelection, true);
        };
    }

    private void SetPanelActive(Panel panel, bool active)
    {
        m_Panels[panel].SetActive(active);
    }

    private void DeactiveAllPanels()
    {
        foreach (var panel in Enum.GetValues(typeof(Panel)))
        {
            SetPanelActive((Panel)panel, false);
        }
    }
}