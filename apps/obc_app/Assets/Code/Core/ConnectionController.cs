using DataLink;
using Microsoft.Win32;
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using System.Text.RegularExpressions;
using UnityEngine;

public class ConnectionController : MonoBehaviour
{
    private const string RP2040_PID = "000A";
    private const string RP2040_VID = "2E8A";

    private const string TCP_DESTINATION = "127.0.0.1:12347";

    public static event EventHandler<DestinationSelectedEventArgs> OnDestinationSelected;

    private void Start()
    {
        StartCoroutine(FetchPorts());
    }

    private IEnumerator FetchPorts()
    {
        yield return new WaitForSeconds(0.1f);

        while (true)
        {
            if (!CommunicationManager.Instance.GetProtocol().IsConnected())
            {
                if (CommunicationManager.Instance.IsSerial())
                {
                    HandleSerialConnection();
                }
                else
                {
                    HandleTCPConnection();
                }
            }

            yield return new WaitForSeconds(0.5f);
        }
    }


    private void HandleSerialConnection()
    {
        var ports = ListSerialPorts();

        print("Fetching serial ports returned: " + ports.Count + " ports");

        if (ports.Count > 0)
        {
            BeginConnect(ports[0]);
        }
    }

    private void HandleTCPConnection()
    {
        if (Input.GetKey(KeyCode.Return))
        {
            BeginConnect(TCP_DESTINATION);
        }
    }

    private void BeginConnect(string dest)
    {
        OnDestinationSelected?.Invoke(this, new DestinationSelectedEventArgs { Destination = dest });

        CommunicationManager.Instance.GetProtocol().Connect(dest);
    }

    /**
     * REF: https://stackoverflow.com/questions/10350340/identify-com-port-using-vid-and-pid-for-usb-device-attached-to-x64
     */
    private List<string> ListSerialPorts()
    {
        var pattern = string.Format("^VID_{0}.PID_{1}", RP2040_VID, RP2040_PID);
        var _rx = new Regex(pattern, RegexOptions.IgnoreCase);
        var comports = new List<string>();

        var rk1 = Registry.LocalMachine;
        var rk2 = rk1.OpenSubKey("SYSTEM\\CurrentControlSet\\Enum");

        foreach (var s3 in rk2.GetSubKeyNames())
        {
            var rk3 = rk2.OpenSubKey(s3);

            foreach (var s in rk3.GetSubKeyNames())
            {
                if (_rx.Match(s).Success)
                {
                    var rk4 = rk3.OpenSubKey(s);

                    foreach (var s2 in rk4.GetSubKeyNames())
                    {
                        var rk5 = rk4.OpenSubKey(s2);
                        var location = (string)rk5.GetValue("LocationInformation");
                        var rk6 = rk5.OpenSubKey("Device Parameters");
                        var portName = (string)rk6.GetValue("PortName");

                        if (!string.IsNullOrEmpty(portName) && SerialPort.GetPortNames().Contains(portName))
                        {
                            comports.Add(portName);
                        }
                    }
                }
            }
        }

        return comports;
    }

    public class DestinationSelectedEventArgs : EventArgs
    {
        public string Destination { get; set; }
    }
}