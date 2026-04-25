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
    PubSub::Publisher<PubSub::Topics::PMUState> m_Publisher{PUBSUB_ID(pmu_state)};
    
    XPowersAXP2101 m_Device;
};