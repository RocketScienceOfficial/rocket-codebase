using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;

public class MainPanelController : MonoBehaviour
{
    [SerializeField] private TextMeshProUGUI m_PortText;

    private void Start()
    {
        ConnectionController.OnDestinationSelected += (sender, args) =>
        {
            m_PortText.SetText(args.Destination);
        };
    }
}