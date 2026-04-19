using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class GaugePanelController : MonoBehaviour
{
    [SerializeField] private TextMeshProUGUI m_ValueText;
    [SerializeField] private Image m_FillImage;

    private float _maxFill;
    private bool _initialized;

    private void Start()
    {
        if (!_initialized)
        {
            m_ValueText.SetText("0");
            m_FillImage.fillAmount = 0;

            Init();
        }
    }

    private void Init()
    {
        if (!_initialized)
        {
            _maxFill = transform.Find("Red Background").GetComponent<Image>().fillAmount;

            _initialized = true;
        }
    }

    public void SetValue(float value, float minValue, float maxValue)
    {
        Init();

        value = Mathf.Clamp(value, minValue, maxValue);

        m_ValueText.SetText(string.Format("{0:0.0}", value).Replace(',', '.'));
        m_FillImage.fillAmount = (value - minValue) / (maxValue - minValue) * _maxFill;
    }

    public void SetValue(int value, int minValue, int maxValue)
    {
        Init();

        value = Mathf.Clamp(value, minValue, maxValue);

        m_ValueText.SetText(value.ToString());
        m_FillImage.fillAmount = (float)(value - minValue) / (maxValue - minValue) * _maxFill;
    }
}