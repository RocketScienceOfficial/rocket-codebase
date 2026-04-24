#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Subscriber.h>
#include <u8g2.h>

class OLEDModule
{
public:
    void init();
    void run();

private:
    u8g2_t m_Display;
    uint32_t m_LastUpdateTime;

    struct PanelData
    {
        const char *name;
        int rx;
        int tx;
        int rssi;
        int execTimeoutLeft;
        double lat;
        double lon;
        int batteryPercentage;
        float batteryVoltage;
    };
    PanelData m_CurrentData;

    enum class OLEDState
    {
        ROCKET,
        GCS,
        _LAST,
    };
    OLEDState m_CurrentState;
    bool m_StateInitialized;
    bool m_ButtonPressed;

    void initDisplay();
    bool shouldChangeState();

    void handleStateChange();
    void setNewState(OLEDState newState);
    void drawPanel();
};