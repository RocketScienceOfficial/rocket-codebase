#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <XPowersAXP2101.h>

class PMUModule
{
public:
    void init();
    void run();

private:
    PubSub::Publisher<PubSub::Topics::pmu_state_topic> m_Publisher;
    
    XPowersAXP2101 m_Device;
};