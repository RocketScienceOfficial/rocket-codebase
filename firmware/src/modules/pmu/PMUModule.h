#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Subscriber.h>
#include <XPowersAXP2101.h>

class PMUModule
{
public:
    void init();
    void run();

private:
    XPowersAXP2101 m_Device;
};