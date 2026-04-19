using System;
using UnityEngine;

public class Watchdog
{
    private readonly string _name;
    private readonly WatchdogTimerMonoBehaviour _gameObject;

    public Watchdog(string name, float maxTime, Action resetAction)
    {
        _name = name;

        _gameObject = new GameObject($"Watchdog Timer ({_name})").AddComponent<WatchdogTimerMonoBehaviour>();
        _gameObject.OnReset = () =>
        {
            Update();

            Debug.Log($"Watchdog ({_name}) was fired!");

            resetAction();
        };
        _gameObject.MaxTime = maxTime;

        Disable();
    }

    public void Enable()
    {
        Debug.Log($"Watchdog ({_name}) enabled!");

        _gameObject.IsEnabled = true;

        Update();
    }

    public void Disable()
    {
        Debug.Log($"Watchdog ({_name}) disabled!");

        _gameObject.IsEnabled = false;

        Update();
    }

    public void Update()
    {
        _gameObject.Timer = 0f;
    }


    public class WatchdogTimerMonoBehaviour : MonoBehaviour
    {
        public Action OnReset { get; set; }
        public float MaxTime { get; set; }
        public bool IsEnabled { get; set; }
        public float Timer { get; set; }

        private void Update()
        {
            if (IsEnabled)
            {
                Timer += Time.deltaTime;

                if (Timer >= MaxTime)
                {
                    OnReset();
                }
            }
        }
    }
}