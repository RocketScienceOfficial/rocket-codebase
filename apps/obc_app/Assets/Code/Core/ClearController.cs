using DataLink;
using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class ClearController : MonoBehaviour
{
    private const float SPEED = 15.0f;

    [SerializeField] private Button m_ClearButton;
    [SerializeField] private Button m_ConfirmButton;
    [SerializeField] private Button m_CancelButton;
    [SerializeField] private Image m_ProgressFill;
    [SerializeField] private TextMeshProUGUI m_ProgressText;
    [SerializeField] private GameObject m_ConfirmPanel;
    [SerializeField] private GameObject m_InfoPanel;

    private bool _isClearing;
    private float _totalPercentage;
    private float _currentProgress;
    private Watchdog _watchdog;

    private void Start()
    {
        _watchdog = new Watchdog("ClearController", 20f, () => FinishClearing());

        m_ClearButton.onClick.AddListener(() =>
        {
            m_ConfirmPanel.SetActive(true);
            m_InfoPanel.SetActive(false);

            PanelsManager.Instance.SetPanelActive(PanelType.Clear, true);
        });

        m_ConfirmButton.onClick.AddListener(() =>
        {
            m_ConfirmPanel.SetActive(false);
            m_InfoPanel.SetActive(true);

            _isClearing = true;
            _totalPercentage = 0;
            _currentProgress = 0;
            _watchdog.Enable();

            UpdateProgress();

            CommunicationManager.Instance.GetProtocol().SendData(new clear_request().Pack());
        });

        m_CancelButton.onClick.AddListener(() =>
        {
            FinishClearing();
        });

        CommunicationManager.Instance.OnRead += (sender, args) =>
        {
            if (_isClearing)
            {
                var msg = args.Message;

                if (msg.msg_id == clear_finish.MSG_ID)
                {
                    FinishClearing();
                }
                else if (msg.msg_id == clear_progress_info.MSG_ID)
                {
                    var payload = clear_progress_info.Unpack(msg);

                    _totalPercentage = payload.percentage;
                    _watchdog.Update();
                }
            }
        };

        CommunicationManager.Instance.OnDisconnected += (sender, args) =>
        {
            if (_isClearing)
            {
                print("Disconnected while clearing. Aborting...");

                FinishClearing();
            }
        };
    }

    private void Update()
    {
        if (_isClearing)
        {
            if (_currentProgress < _totalPercentage)
            {
                _currentProgress += Time.deltaTime * SPEED;
                _currentProgress = Mathf.Clamp(_currentProgress, 0, 100);

                UpdateProgress();
            }
        }
    }

    private void FinishClearing()
    {
        _isClearing = false;
        _totalPercentage = 0;
        _currentProgress = 0;
        _watchdog.Disable();

        PanelsManager.Instance.SetPanelActive(PanelType.Clear, false);
    }

    private void UpdateProgress()
    {
        m_ProgressFill.fillAmount = _currentProgress / 100.0f;
        m_ProgressText.SetText($"{(int)_currentProgress}%");
    }
}