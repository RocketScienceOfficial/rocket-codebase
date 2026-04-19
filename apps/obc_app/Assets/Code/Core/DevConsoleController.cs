using System;
using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class DevConsoleController : MonoBehaviour
{
    [SerializeField] private GameObject m_ConsoleObject;
    [SerializeField] private Transform m_LogParent;
    [SerializeField] private GameObject m_LogObject;
    [SerializeField] private Button m_CopyButton;

    private readonly List<string> _logs = new();

    private void Start()
    {
        m_ConsoleObject.SetActive(false);

        m_CopyButton.onClick.AddListener(() =>
        {
            var data = "";

            foreach (var log in _logs)
            {
                data += $"{log}\n";
            }

            GUIUtility.systemCopyBuffer = data;
        });
    }

    private void OnEnable()
    {
        Application.logMessageReceived += OnLog;
    }

    private void OnDisable()
    {
        Application.logMessageReceived -= OnLog;
    }

    private void Update()
    {
        if (Input.GetKeyDown(KeyCode.BackQuote))
        {
            m_ConsoleObject.SetActive(!m_ConsoleObject.activeSelf);
        }
    }

    private void OnLog(string condition, string stackTrace, LogType type)
    {
        var obj = Instantiate(m_LogObject, m_LogParent);
        var txt = obj.GetComponentInChildren<TextMeshProUGUI>();
        var logCond = $"[{DateTime.Now:HH:mm:ss}] {condition}";

        txt.SetText(logCond);
        txt.color = type == LogType.Log ? Color.white : Color.red;

        _logs.Add(logCond);
    }
}