using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;
using System.Linq;

public class KMLFile
{
    private readonly List<KMLData> _kmlData = new();
    private StreamWriter _writer;

    public void Open(string path)
    {
        EnsureDirectoryExists(path);

        _writer = new StreamWriter(path);

        Debug.Log($"{path} has been opened!");
    }

    public void Close()
    {
        if (_writer == null)
        {
            return;
        }

        var kmlStr = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n<kml xmlns=\"http://www.opengis.net/kml/2.2\">\r\n  <Document>\r\n    <Style id=\"s1\">\r\n      <LineStyle>\r\n        <color>ffffffff</color>\r\n        <width>2.5</width>\r\n      </LineStyle>\r\n    </Style>\r\n    <Style id=\"s2\">\r\n      <LineStyle>\r\n        <color>00000000</color>\r\n        <width>2.5</width>\r\n      </LineStyle>\r\n      <PolyStyle>\r\n        <color>7f171717</color>\r\n      </PolyStyle>\r\n    </Style>\r\n    <Placemark>\r\n      <styleUrl>#s1</styleUrl>\r\n      <LineString>\r\n        <extrude>0</extrude>\r\n        <altitudeMode>relativeToGround</altitudeMode>\r\n        <coordinates>{DATA}</coordinates>\r\n      </LineString>\r\n    </Placemark>\r\n    <Placemark>\r\n      <styleUrl>#s2</styleUrl>\r\n      <LineString>\r\n        <extrude>1</extrude>\r\n        <altitudeMode>relativeToGround</altitudeMode>\r\n        <coordinates>{DATA}</coordinates>\r\n      </LineString>\r\n    </Placemark>\r\n  </Document>\r\n</kml>";
        var newKml = kmlStr.Replace("{DATA}", string.Join("", _kmlData.Select(d => $"\r\n            {d.lon.ToString().Replace(',', '.')},{d.lat.ToString().Replace(',', '.')},{d.alt.ToString().Replace(',', '.')}")));

        _kmlData.Clear();

        _writer.Write(newKml);
        _writer.Close();
        _writer = null;

        Debug.Log($"File has been closed");
    }

    public void AddRecord(double lat, double lon, float alt)
    {
        var currentKMLData = new KMLData()
        {
            lat = lat,
            lon = lon,
            alt = alt,
            altDiv = 1,
        };

        if (_kmlData.Count > 0)
        {
            var lastData = _kmlData[^1];

            if (lastData.lat == currentKMLData.lat && lastData.lon == currentKMLData.lon)
            {
                lastData.alt = (lastData.alt * lastData.altDiv + currentKMLData.alt) / (lastData.altDiv + 1);
                lastData.altDiv++;

                _kmlData[^1] = lastData;
            }
            else
            {
                _kmlData.Add(currentKMLData);
            }
        }
        else
        {
            _kmlData.Add(currentKMLData);
        }
    }

    private void EnsureDirectoryExists(string filePath)
    {
        var fi = new FileInfo(filePath);

        if (!fi.Directory.Exists)
        {
            Directory.CreateDirectory(fi.DirectoryName);
        }
    }

    private struct KMLData
    {
        public double lat;
        public double lon;
        public float alt;
        public int altDiv;
    }
}