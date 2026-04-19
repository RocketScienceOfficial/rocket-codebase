using DataLink;
using System;
using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class DownloadController : MonoBehaviour
{
    [SerializeField] private Button m_DownloadButton;
    [SerializeField] private Button m_RecoveryButton;
    [SerializeField] private Image m_ProgressFill;
    [SerializeField] private TextMeshProUGUI m_ProgressText;

    private readonly CSVFile _csv = new();
    private readonly KMLFile _kml = new();
    private Watchdog _watchdog;
    private bool _modeDownload;
    private bool _isDownloading;
    private bool _downloadInitialized;
    private int _currentCount;
    private int _totalCount;

    private void Start()
    {
        _watchdog = new Watchdog("DownloadController", 1f, () => FinishDownloading());

        m_DownloadButton.onClick.AddListener(() =>
        {
            _modeDownload = true;

            BeginDownloading();

            CommunicationManager.Instance.GetProtocol().SendData(new read_request().Pack());
        });

        m_RecoveryButton.onClick.AddListener(() =>
        {
            _modeDownload = false;

            BeginDownloading();

            CommunicationManager.Instance.GetProtocol().SendData(new recovery_request().Pack());
        });

        CommunicationManager.Instance.OnRead += (sender, args) =>
        {
            if (_isDownloading)
            {
                var msg = args.Message;

                if (msg.msg_id == saved_data_chunk_obc.MSG_ID)
                {
                    HandleNewFrame(true, msg);
                }
                else if (msg.msg_id == saved_data_chunk_acs.MSG_ID)
                {
                    HandleNewFrame(false, msg);
                }
                else if (msg.msg_id == saved_data_chunk_size.MSG_ID)
                {
                    if (_modeDownload)
                    {
                        var payload = saved_data_chunk_size.Unpack(msg);

                        _totalCount = (int)payload.size;
                        _watchdog.Update();

                        print("New total count is: " + _totalCount);

                        if (_totalCount == 0)
                        {
                            print("Received count is 0. Aborting...");

                            FinishDownloading();
                        }
                        else
                        {
                            UpdateProgress();
                        }
                    }
                }
                else if (msg.msg_id == read_finish.MSG_ID)
                {
                    FinishDownloading();
                }
                else if (msg.msg_id == recovery_finish.MSG_ID)
                {
                    FinishDownloading();
                }
            }
        };

        CommunicationManager.Instance.OnDisconnected += (sender, args) =>
        {
            if (_isDownloading)
            {
                print("Disconnected while downloading. Aborting...");

                FinishDownloading();
            }
        };
    }

    private void BeginDownloading()
    {
        _isDownloading = true;
        _downloadInitialized = false;

        _watchdog.Enable();

        UpdateProgress();

        PanelsManager.Instance.SetPanelActive(PanelType.Download, true);
    }

    private void FinishDownloading()
    {
        _csv.Close();
        _kml.Close();
        _watchdog.Disable();

        _isDownloading = false;
        _currentCount = 0;
        _totalCount = 0;

        PanelsManager.Instance.SetPanelActive(PanelType.Download, false);

        print("Finished downloading data!");
    }

    private void UpdateProgress()
    {
        if (_modeDownload)
        {
            m_ProgressFill.fillAmount = _isDownloading && _totalCount > 0 ? (float)_currentCount / _totalCount : 0;
            m_ProgressText.SetText(_totalCount != 0 ? $"{_currentCount} / {_totalCount}" : "Loading...");
        }
        else
        {
            m_ProgressFill.fillAmount = 0;
            m_ProgressText.SetText($"{_currentCount} frames recovered");
        }
    }

    private void HandleNewFrame(bool obc, datalink_message msg)
    {
        if (!_downloadInitialized)
        {
            _csv.Open($"Downloads/FlightLog_{DateTime.Now:yyyy-MM-dd--HH-mm-ss}.csv");
            _kml.Open($"Downloads/FlightKML_{DateTime.Now:yyyy-MM-dd--HH-mm-ss}.kml");

            WriteCSVHeader(obc, _csv);

            _downloadInitialized = true;
        }

        ProcessFrame(obc, msg, _csv, _kml);

        _currentCount++;
        _watchdog.Update();

        UpdateProgress();

        if (_modeDownload && _totalCount == _currentCount)
        {
            print("Received all frames!");
        }
    }

    private void WriteCSVHeader(bool obc, CSVFile csv)
    {
        if (obc)
        {
            WriteCSVHeader_OBC(csv);
        }
        else
        {
            WriteCSVHeader_ACS(csv);
        }

        csv.EndLine();
    }

    private void ProcessFrame(bool obc, datalink_message msg, CSVFile csv, KMLFile kml)
    {
        if (obc)
        {
            var payload = saved_data_chunk_obc.Unpack(msg);

            WriteCSVFrame_OBC(csv, payload);

            kml.AddRecord(payload.lat, payload.lon, (float)payload.alt);
        }
        else
        {
            var payload = saved_data_chunk_acs.Unpack(msg);

            WriteCSVFrame_ACS(csv, payload);

            kml.AddRecord(payload.lat, payload.lon, (float)payload.alt);
        }

        csv.EndLine();
    }

    private void WriteCSVHeader_OBC(CSVFile csv)
    {
        csv.WriteString("dt");
        csv.WriteString("acc_rawX");
        csv.WriteString("acc_rawY");
        csv.WriteString("acc_rawZ");
        csv.WriteString("gyro_rawX");
        csv.WriteString("gyro_rawY");
        csv.WriteString("gyro_rawZ");
        csv.WriteString("mag_rawX");
        csv.WriteString("mag_rawY");
        csv.WriteString("mag_rawZ");
        csv.WriteString("lat");
        csv.WriteString("lon");
        csv.WriteString("alt");
        csv.WriteString("gps_fix");
        csv.WriteString("gps_sats");
        csv.WriteString("pressure");
        csv.WriteString("acc_N");
        csv.WriteString("acc_E");
        csv.WriteString("acc_D");
        csv.WriteString("vel_N");
        csv.WriteString("vel_E");
        csv.WriteString("vel_D");
        csv.WriteString("pos_N");
        csv.WriteString("pos_E");
        csv.WriteString("pos_D");
        csv.WriteString("rot_x");
        csv.WriteString("rot_y");
        csv.WriteString("rot_z");
        csv.WriteString("state");
        csv.WriteString("battery");
        csv.WriteString("ign1_cont");
        csv.WriteString("ign2_cont");
        csv.WriteString("ign3_cont");
        csv.WriteString("ign4_cont");
        csv.WriteString("ign1_fired");
        csv.WriteString("ign2_fired");
        csv.WriteString("ign3_fired");
        csv.WriteString("ign4_fired");
    }

    private void WriteCSVHeader_ACS(CSVFile csv)
    {
        csv.WriteString("dt");
        csv.WriteString("acc_rawX");
        csv.WriteString("acc_rawY");
        csv.WriteString("acc_rawZ");
        csv.WriteString("gyro_rawX");
        csv.WriteString("gyro_rawY");
        csv.WriteString("gyro_rawZ");
        csv.WriteString("mag_rawX");
        csv.WriteString("mag_rawY");
        csv.WriteString("mag_rawZ");
        csv.WriteString("lat");
        csv.WriteString("lon");
        csv.WriteString("alt");
        csv.WriteString("gps_fix");
        csv.WriteString("gps_sats");
        csv.WriteString("pressure");
        csv.WriteString("acc_N");
        csv.WriteString("acc_E");
        csv.WriteString("acc_D");
        csv.WriteString("vel_N");
        csv.WriteString("vel_E");
        csv.WriteString("vel_D");
        csv.WriteString("pos_N");
        csv.WriteString("pos_E");
        csv.WriteString("pos_D");
        csv.WriteString("rot_x");
        csv.WriteString("rot_y");
        csv.WriteString("rot_z");
        csv.WriteString("state");
        csv.WriteString("battery");
        csv.WriteString("setpoint");
        csv.WriteString("roll");
        csv.WriteString("pidOutputAngle");
    }

    private static void WriteCSVFrame_OBC(CSVFile csv, saved_data_chunk_obc payload)
    {
        var quat = new Quaternion(payload.qx, payload.qy, payload.qz, payload.qw);

        csv.WriteFileValue(payload.dt);
        csv.WriteFileValue(payload.accRawX);
        csv.WriteFileValue(payload.accRawY);
        csv.WriteFileValue(payload.accRawZ);
        csv.WriteFileValue(payload.gyroRawX);
        csv.WriteFileValue(payload.gyroRawY);
        csv.WriteFileValue(payload.gyroRawZ);
        csv.WriteFileValue(payload.magRawX);
        csv.WriteFileValue(payload.magRawY);
        csv.WriteFileValue(payload.magRawZ);
        csv.WriteFileValue(payload.lat);
        csv.WriteFileValue(payload.lon);
        csv.WriteFileValue(payload.alt);
        csv.WriteFileValue(payload.gpsData & 0x01);
        csv.WriteFileValue(payload.gpsData >> 1);
        csv.WriteFileValue(payload.pressure);
        csv.WriteFileValue(payload.accN);
        csv.WriteFileValue(payload.accE);
        csv.WriteFileValue(payload.accD);
        csv.WriteFileValue(payload.velN);
        csv.WriteFileValue(payload.velE);
        csv.WriteFileValue(payload.velD);
        csv.WriteFileValue(payload.posN);
        csv.WriteFileValue(payload.posE);
        csv.WriteFileValue(payload.posD);
        csv.WriteFileValue(quat.eulerAngles.x);
        csv.WriteFileValue(quat.eulerAngles.y);
        csv.WriteFileValue(quat.eulerAngles.z);
        csv.WriteFileValue(payload.smState);
        csv.WriteFileValue(payload.batteryVoltage100 / 100.0f);

        for (int i = 0; i < 8; i++)
        {
            var flag = (payload.ignFlags & (1 << i)) >> i;

            csv.WriteFileValue(flag);
        }
    }

    private static void WriteCSVFrame_ACS(CSVFile csv, saved_data_chunk_acs payload)
    {
        var quat = new Quaternion(payload.qx, payload.qy, payload.qz, payload.qw);

        csv.WriteFileValue(payload.dt);
        csv.WriteFileValue(payload.accRawX);
        csv.WriteFileValue(payload.accRawY);
        csv.WriteFileValue(payload.accRawZ);
        csv.WriteFileValue(payload.gyroRawX);
        csv.WriteFileValue(payload.gyroRawY);
        csv.WriteFileValue(payload.gyroRawZ);
        csv.WriteFileValue(payload.magRawX);
        csv.WriteFileValue(payload.magRawY);
        csv.WriteFileValue(payload.magRawZ);
        csv.WriteFileValue(payload.lat);
        csv.WriteFileValue(payload.lon);
        csv.WriteFileValue(payload.alt);
        csv.WriteFileValue(payload.gpsData & 0x01);
        csv.WriteFileValue(payload.gpsData >> 1);
        csv.WriteFileValue(payload.pressure);
        csv.WriteFileValue(payload.accN);
        csv.WriteFileValue(payload.accE);
        csv.WriteFileValue(payload.accD);
        csv.WriteFileValue(payload.velN);
        csv.WriteFileValue(payload.velE);
        csv.WriteFileValue(payload.velD);
        csv.WriteFileValue(payload.posN);
        csv.WriteFileValue(payload.posE);
        csv.WriteFileValue(payload.posD);
        csv.WriteFileValue(quat.eulerAngles.x);
        csv.WriteFileValue(quat.eulerAngles.y);
        csv.WriteFileValue(quat.eulerAngles.z);
        csv.WriteFileValue(payload.smState);
        csv.WriteFileValue(payload.batteryVoltage100 / 100.0f);
        csv.WriteFileValue(payload.angleSetpoint10 / 10.0f);
        csv.WriteFileValue(payload.pidRoll10 / 10.0f);
        csv.WriteFileValue(payload.pidOutputAngle10 / 10.0f);
    }
}