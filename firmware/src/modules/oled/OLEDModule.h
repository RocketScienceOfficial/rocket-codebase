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
    PubSub::Subscriber<PubSub::Topics::pmu_state_topic> m_PMUSubscriber;
    PubSub::Subscriber<PubSub::Topics::sensors_simplified_gps_1_topic> m_SimplifiedGPSSubscriber;
    PubSub::Subscriber<PubSub::Topics::gcs_commander_timeout_topic> m_GCSCommanderTimeoutSubscriber;
    PubSub::Subscriber<PubSub::Topics::lora_rx_topic> m_RadioSubscriber;
    PubSub::Subscriber<PubSub::Topics::gcs_radio_state_topic> m_GCSRadioStateSubscriber;

    u8g2_t m_Display;
    uint32_t m_LogoDisableTime;
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
    PanelData m_RocketData;
    PanelData m_GCSData;
    const PanelData *m_CurrentData;

    enum class OLEDState
    {
        ROCKET,
        GCS,
        LAST_DO_NOT_USE,
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