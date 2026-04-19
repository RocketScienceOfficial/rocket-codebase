using DataLink;
using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;

public class SignalPanelController : MonoBehaviour
{
    private const float SIGNAL_TIMEOUT = 5.0f;
    private const int PLOT_SIZE = 400;
    private const float PLOT_MOVE_TIME = 0.01f;
    private const float PLOT_VERTEX_SPACING = 0.01f;
    private const float PLOT_MIN_VALUE = -120.0f;
    private const float PLOT_MAX_VALUE = -10.0f;
    private const float PLOT_MIN_SCALE = 0.0f;
    private const float PLOT_MAX_SCALE = 4.0f;

    [SerializeField] private TextMeshProUGUI m_RSSIText;
    [SerializeField] private TextMeshProUGUI m_RXTXText;
    [SerializeField] private TextMeshProUGUI m_PacketLossText;
    [SerializeField] private TextMeshProUGUI m_RateText;
    [SerializeField] private MeshFilter m_PlotMeshFilter;

    private float _lastPacketTime;
    private float _currentPlotValue;
    private float _plotMoveTimer;
    private readonly float[] _plotVals = new float[PLOT_SIZE];
    private readonly Vector3[] _plotVertices = new Vector3[PLOT_SIZE * 2];
    private readonly int[] _plotIndices = new int[(PLOT_SIZE * 2 - 2) * 3];
    private readonly Vector2[] _plotUvs = new Vector2[PLOT_SIZE * 2];
    private readonly Color[] _plotVerticesColors = new Color[PLOT_SIZE * 2];
    private Mesh _plotMesh;

    private void Start()
    {
        InitPlot();
        ResetPlot();

        CommunicationManager.Instance.OnConnected += (sender, args) =>
        {
            SetTextUI(int.MinValue, 0, 0, 100, 0);
            ResetPlot();

            _lastPacketTime = 0f;
        };

        CommunicationManager.Instance.OnDisconnected += (sender, args) =>
        {
            _lastPacketTime = 0f;
        };

        CommunicationManager.Instance.OnRead += (sender, args) =>
        {
            var msg = args.Message;

            if (msg.msg_id == telemetry_data_gcs.MSG_ID)
            {
                var payload = telemetry_data_gcs.Unpack(msg);

                var signalStrength = Mathf.Clamp(-payload.signalStrengthNeg, PLOT_MIN_VALUE, PLOT_MAX_VALUE);
                var packetLoss = payload.packetLossPercentage;
                var rate = 1f / (Time.time - _lastPacketTime);

                SetTextUI((int)signalStrength, payload.packetsReceived, payload.packetsTransmitted, packetLoss, rate);

                _lastPacketTime = Time.time;
                _currentPlotValue = signalStrength;
            }
        };
    }

    private void Update()
    {
        HandleTimeout();
        UpdatePlot();
    }

    private void InitPlot()
    {
        for (var i = 0; i < PLOT_SIZE; i++)
        {
            var vertInd = i * 2;

            if (i > 0)
            {
                var triInd = (i - 1) * 6;

                _plotIndices[triInd + 0] = vertInd;
                _plotIndices[triInd + 1] = vertInd - 1;
                _plotIndices[triInd + 2] = vertInd - 2;

                _plotIndices[triInd + 3] = vertInd;
                _plotIndices[triInd + 4] = vertInd + 1;
                _plotIndices[triInd + 5] = vertInd - 1;
            }
        }

        for (var i = 0; i < _plotUvs.Length; i++)
        {
            _plotUvs[i] = new Vector2(0.5f, 0.5f);
            _plotVerticesColors[i] = new Color(0.0f, 0.0f, 0.0f, 1.0f);
        }

        _plotMesh = new Mesh();
        m_PlotMeshFilter.mesh = _plotMesh;
    }

    private void DrawPlot()
    {
        for (int i = 0; i < _plotVals.Length; i++)
        {
            var vertInd = i * 2;
            var convertedVal = (PLOT_MAX_SCALE - PLOT_MIN_SCALE) / (PLOT_MAX_VALUE - PLOT_MIN_VALUE) * (_plotVals[i] - PLOT_MIN_VALUE) + PLOT_MIN_SCALE;

            _plotVertices[vertInd + 0] = new Vector3(i * PLOT_VERTEX_SPACING, convertedVal, 0.0f);
            _plotVertices[vertInd + 1] = new Vector3(i * PLOT_VERTEX_SPACING, 0.0f, 0.0f);
        }

        _plotMesh.Clear();

        _plotMesh.vertices = _plotVertices;
        _plotMesh.uv = _plotUvs;
        _plotMesh.triangles = _plotIndices;
        _plotMesh.colors = _plotVerticesColors;

        _plotMesh.Optimize();
        _plotMesh.RecalculateNormals();
    }

    private void HandleTimeout()
    {
        if (_lastPacketTime != 0f && Time.time - _lastPacketTime >= SIGNAL_TIMEOUT)
        {
            print("Signal timeout detected!");

            SetTextUI(int.MinValue, -1, -1, 100, 0);

            _lastPacketTime = 0f;
            _currentPlotValue = PLOT_MIN_VALUE;
        }
    }

    private void UpdatePlot()
    {
        _plotMoveTimer -= Time.deltaTime;

        if (_plotMoveTimer <= 0f)
        {
            _plotMoveTimer = PLOT_MOVE_TIME;

            for (int i = 1; i < _plotVals.Length; i++)
            {
                _plotVals[i - 1] = _plotVals[i];
            }

            _plotVals[^1] = _currentPlotValue;

            DrawPlot();
        }
    }

    private void ResetPlot()
    {
        _currentPlotValue = PLOT_MIN_VALUE;

        for (int i = 0; i < _plotVals.Length; i++)
        {
            _plotVals[i] = _currentPlotValue;
        }
    }

    private void SetTextUI(int rssi, int rx, int tx, int packetLossPercentage, float rate)
    {
        m_RSSIText.SetText(rssi != int.MinValue ? $"{rssi} dbm" : "No Signal");

        if (rx >= 0 && tx >= 0)
        {
            m_RXTXText.SetText($"{rx} / {tx}");
        }

        m_PacketLossText.SetText($"{packetLossPercentage}% Packet Loss");
        m_RateText.SetText(string.Format("{0:0.0}", rate).Replace(',', '.') + " Packets/s");
    }
}