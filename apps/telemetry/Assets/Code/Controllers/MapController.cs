using DataLink;
using System;
using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.Networking;
using UnityEngine.UI;

// REF: https://wiki.openstreetmap.org/wiki/Slippy_map_tilenames
// REF: https://operations.osmfoundation.org/policies/tiles/
// REF: https://en.wikipedia.org/wiki/Web_Mercator_projection

public class MapController : MonoBehaviour
{
    private const int MAP_ZOOM = 16;
    private const float MAP_UPDATE_RATE = 5f;

    [SerializeField] private GameObject m_NoDataPanel;
    [SerializeField] private Button m_OpenGoogleMapsButton;
    [SerializeField] private TextMeshProUGUI m_LatitudeText;
    [SerializeField] private TextMeshProUGUI m_LongtitudeText;
    [SerializeField] private TextMeshProUGUI m_FixTypeText;
    [SerializeField] private TextMeshProUGUI m_SVCountText;
    [SerializeField] private RectTransform m_RocketMarker;
    [SerializeField] private RectTransform m_GCSMarker;
    [SerializeField] private RawImage[] m_Tiles;

    private float _lastUpdateTime;
    private double _lastLat;
    private double _lastLon;
    private Vector2Int _currentCenter;

    private void Start()
    {
        m_OpenGoogleMapsButton.onClick.AddListener(() =>
        {
            if (_lastLat != 0 && _lastLon != 0)
            {
                print($"Opening Google Maps for coordinates: {_lastLat}, {_lastLon}");

                Application.OpenURL($"https://maps.google.com?q={_lastLat},{_lastLon}");
            }
        });

        CommunicationManager.Instance.OnConnected += (sender, args) =>
        {
            m_NoDataPanel.SetActive(true);

            SetTextUI(0, 0, false, 0);

            _lastUpdateTime = 0;
            _lastLat = 0;
            _lastLon = 0;
            _currentCenter = Vector2Int.zero;
        };

        CommunicationManager.Instance.OnRead += (sender, args) =>
        {
            var msg = args.Message;

            if (msg.msg_id == telemetry_data_gcs.MSG_ID)
            {
                var payload = telemetry_data_gcs.Unpack(msg);

                var fix3d = (payload.gpsData & 0x01) != 0;
                var svCount = payload.gpsData >> 1;
                
                StartCoroutine(UpdateMap(payload.lat / 10000000.0, payload.lon / 10000000.0, payload.gcsLat / 10000000.0, payload.gcsLon / 10000000.0, fix3d, svCount));
            }
        };
    }

    private IEnumerator UpdateMap(double lat, double lon, double gcsLat, double gcsLon, bool fix3d, int svCount)
    {
        if (lat != 0 && lon != 0)
        {
            var pos = GetTileXY(lat, lon);

            if ((Mathf.Abs(pos.x - _currentCenter.x) >= 2 || Mathf.Abs(pos.y - _currentCenter.y) >= 2) && (Time.time - MAP_UPDATE_RATE >= _lastUpdateTime || _lastUpdateTime == 0))
            {
                yield return SetTiles(pos);

                _lastUpdateTime = Time.time;
                _currentCenter = pos;
            }

            SetTextUI(lat, lon, fix3d, svCount);
            SetMarkerLocation(m_RocketMarker, _currentCenter, lat, lon);
            SetMarkerLocation(m_GCSMarker, _currentCenter, gcsLat, gcsLon);

            _lastLat = lat;
            _lastLon = lon;
        }
    }

    private IEnumerator SetTiles(Vector2Int center)
    {
        var index = 0;

        for (int h = -1; h <= 1; h++)
        {
            for (int w = -2; w <= 2; w++)
            {
                var url = $"https://tile.openstreetmap.org/{MAP_ZOOM}/{center.x + w}/{center.y + h}.png";

                using var map = UnityWebRequestTexture.GetTexture(url);

                yield return map.SendWebRequest();

                if (map.result == UnityWebRequest.Result.ConnectionError || map.result == UnityWebRequest.Result.ProtocolError)
                {
                    Debug.LogError("Map error: " + map.error);
                }

                var content = DownloadHandlerTexture.GetContent(map);

                m_Tiles[index++].texture = content;
            }
        }

        m_NoDataPanel.SetActive(false);

        print("Maps have been updated!");
    }

    private void SetMarkerLocation(RectTransform marker, Vector2Int center, double lat, double lon)
    {
        var x = GetWebMercatorX(lon) - center.x;
        var y = GetWebMercatorY(lat) - center.y;
        var cellSize = m_Tiles[0].transform.parent.GetComponent<GridLayoutGroup>().cellSize.x;

        x = (2 * x - 1) * cellSize / 2.0;
        y = (1 - 2 * y) * cellSize / 2.0;

        var rect = marker.parent.GetComponent<RectTransform>().rect;

        marker.gameObject.SetActive(x >= -rect.width / 2 && x <= rect.width / 2 && y >= -rect.height / 2 && y <= rect.height / 2);
        marker.anchoredPosition = new Vector2((float)x, (float)y);

        print("Marker position has been set!");
    }

    private Vector2Int GetTileXY(double lat, double lon)
    {
        var x = (int)Math.Floor(GetWebMercatorX(lon));
        var y = (int)Math.Floor(GetWebMercatorY(lat));

        return new Vector2Int(x, y);
    }

    private double GetWebMercatorX(double lon)
    {
        return (lon + 180.0) / 360.0 * Math.Pow(2, MAP_ZOOM);
    }

    private double GetWebMercatorY(double lat)
    {
        return (1 - Math.Log(Math.Tan(lat * Math.PI / 180.0) + 1 / Math.Cos(lat * Math.PI / 180.0)) / Math.PI) * Math.Pow(2, MAP_ZOOM - 1);
    }

    private void SetTextUI(double lat, double lon, bool fix3d, int svCount)
    {
        m_LatitudeText.SetText(string.Format("{0:0.0000000}", lat).Replace(',', '.'));
        m_LongtitudeText.SetText(string.Format("{0:0.0000000}", lon).Replace(',', '.'));
        m_SVCountText.SetText($"{svCount}");
        m_FixTypeText.SetText(fix3d ? "3D" : "2D");
        m_FixTypeText.color = fix3d ? Color.green : Color.red;
    }
}