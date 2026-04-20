#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <osal/systime.h>
#include <lib/debug/sys_assert.h>

template <typename Derived, typename TopicStruct>
class DriverBase
{
public:
    DriverBase(const PubSub::TopicMetadata<TopicStruct> *meta) : m_Publisher(meta), m_ReadDelay(0) {}
    DriverBase(const PubSub::TopicMetadata<TopicStruct> *meta, int frequency) : m_Publisher(meta), m_ReadDelay(1000 / frequency)
    {
        SYS_ASSERT(frequency > 0 && frequency <= 1000);
    }

    void init()
    {
        m_Driver = static_cast<Derived *>(this);
        m_Driver->initialize();
    }

    void update()
    {
        if (m_ReadDelay == 0 || osal_systime_get_ms() - m_LastPublishTime >= m_ReadDelay)
        {
            float dt = (osal_systime_get_ms() - m_LastPublishTime) / 1000.0f;

            SYS_ASSERT(m_Driver != nullptr);

            m_Driver->readAndPublish(dt);
            m_LastPublishTime = osal_systime_get_ms();
        }
    }

protected:
    TopicStruct m_CurrentFrame;
    PubSub::Publisher<TopicStruct> m_Publisher;

private:
    Derived *m_Driver;
    uint32_t m_ReadDelay = 0;
    uint32_t m_LastPublishTime = 0;
};