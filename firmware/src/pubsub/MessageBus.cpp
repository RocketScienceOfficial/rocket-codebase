#include "MessageBus.h"
#include <lib/debug/sys_assert.h>
#include <cstring>
#include <atomic>

namespace PubSub
{
    struct Topic
    {
        TopicId id;
        const TopicHandle *owner;
        const char *name;
        size_t memory_offset;
        size_t message_size;
        size_t message_count;
        std::atomic<uint32_t> write_sequence;
    };

    static bool g_creationBlocked = false;
    static uint8_t g_topicsMemoryBuffer[32 * 1024];
    static size_t g_nextFreeMemoryOffset = 0;
    static Topic g_topics[MAX_TOPICS];
    static uint8_t g_topicsCount = 0;

    static Topic *FindTopicById(TopicId id)
    {
        for (size_t i = 0; i < MAX_TOPICS; ++i)
        {
            if (g_topics[i].message_size > 0 && g_topics[i].id == id)
            {
                return &g_topics[i];
            }
        }

        return nullptr;
    }

    static bool ValidateHandle(const TopicHandle *handle)
    {
        return handle != nullptr && handle->topic_index < MAX_TOPICS;
    }

    static bool ValidateTopicByHandle(const TopicHandle *handle, const Topic *topic)
    {
        return handle != nullptr && topic != nullptr && topic->message_size > 0;
    }

    void MessageBus::Init()
    {
        g_creationBlocked = true;
    }

    TopicHandle MessageBus::CreateHandle(TopicId id, const char *topic_name, size_t message_size)
    {
        SYS_ASSERT(!g_creationBlocked);
        SYS_ASSERT(id != nullptr);
        SYS_ASSERT(topic_name != nullptr);
        SYS_ASSERT_MSG(message_size > 0 && message_size <= MAX_MESSAGE_SIZE, "Invalid message size: %zu (%s)", message_size, topic_name);

        Topic *t = FindTopicById(id);

        if (t != nullptr)
        {
            SYS_ASSERT_MSG(strcmp(t->name, topic_name) == 0, "Topic ID '%s' already exists with a different name: '%s' (requested: '%s')", topic_name, t->name, topic_name);
            SYS_ASSERT_MSG(t->message_size == message_size, "Topic '%s' already exists with a different message size: %zu (requested: %zu)", topic_name, t->message_size, message_size);

            return {(uint8_t)(t - g_topics), t->write_sequence.load(std::memory_order_acquire)};
        }
        else
        {
            SYS_ASSERT_MSG(g_topicsCount < MAX_TOPICS, "Maximum number of topics reached");

            Topic *slot = &g_topics[g_topicsCount++];

            slot->id = id;
            slot->owner = nullptr;
            slot->name = topic_name;
            slot->memory_offset = 0;
            slot->message_size = message_size;
            slot->message_count = 0;
            slot->write_sequence.store(0, std::memory_order_release);

            return {(uint8_t)(g_topicsCount - 1), slot->write_sequence.load(std::memory_order_acquire)};
        }
    }

    void MessageBus::AdvertiseTopic(const TopicHandle *handle, size_t message_count)
    {
        SYS_ASSERT(!g_creationBlocked);
        SYS_ASSERT(ValidateHandle(handle));

        Topic *topic = &g_topics[handle->topic_index];

        SYS_ASSERT(ValidateTopicByHandle(handle, topic));
        SYS_ASSERT(topic->write_sequence.load(std::memory_order_acquire) == handle->read_sequence);
        SYS_ASSERT_MSG(topic->owner == nullptr, "Topic '%s' is already owned by another handle", topic->name);

        topic->owner = handle;

        SYS_ASSERT_MSG(message_count > 1 && message_count <= MAX_MESSAGE_COUNT && FAST_MODULO(message_count, message_count) == 0, "Invalid message count: %zu (%s)", message_count, topic->name);
        SYS_ASSERT_MSG(g_nextFreeMemoryOffset + topic->message_size * topic->message_count <= sizeof(g_topicsMemoryBuffer), "Not enough memory for topic '%s' (Current: %zu)", topic->name, g_nextFreeMemoryOffset);

        topic->memory_offset = g_nextFreeMemoryOffset;
        topic->message_count = message_count;
        g_nextFreeMemoryOffset += topic->message_size * topic->message_count;
    }

    void MessageBus::PublishData(TopicHandle *handle, const void *data, size_t size)
    {
        SYS_ASSERT(ValidateHandle(handle));
        SYS_ASSERT(data != nullptr);

        Topic *topic = &g_topics[handle->topic_index];

        SYS_ASSERT(ValidateTopicByHandle(handle, topic));
        SYS_ASSERT(topic->write_sequence.load(std::memory_order_acquire) == handle->read_sequence);
        SYS_ASSERT_MSG(size == topic->message_size, "Data size does not match topic message size: %zu (expected: %zu)", size, topic->message_size);
        SYS_ASSERT_MSG(topic->owner == handle, "Handle does not own the topic '%s'", topic->name);

        size_t offset = topic->memory_offset + (FAST_MODULO(topic->write_sequence.load(std::memory_order_acquire), topic->message_count)) * topic->message_size;

        memcpy(g_topicsMemoryBuffer + offset, data, size);

        uint32_t old_seq = topic->write_sequence.fetch_add(1, std::memory_order_release);
        handle->read_sequence = old_seq + 1;
    }

    bool MessageBus::CopyData(TopicHandle *handle, void *buffer, size_t buffer_size, bool latest)
    {
        SYS_ASSERT(ValidateHandle(handle));
        SYS_ASSERT(buffer != nullptr);

        Topic *topic = &g_topics[handle->topic_index];

        SYS_ASSERT(ValidateTopicByHandle(handle, topic));
        SYS_ASSERT_MSG(topic->owner != nullptr, "Topic '%s' is not owned by any handle", topic->name);
        SYS_ASSERT_MSG(buffer_size == topic->message_size, "Buffer size does not match topic message size: %zu (expected: %zu)", buffer_size, topic->message_size);

        uint32_t write_seq = 0;

        do
        {
            write_seq = topic->write_sequence.load(std::memory_order_acquire);

            if (handle->read_sequence == write_seq)
            {
                return false;
            }

            if (latest)
            {
                handle->read_sequence = write_seq - 1;
            }
            else
            {
                if (write_seq - handle->read_sequence > topic->message_count)
                {
                    SYS_ASSERT_MSG(false, "Subscriber is too slow and has missed messages on topic '%s' (read_sequence: %u, write_sequence: %u)", topic->name, handle->read_sequence, write_seq);

                    handle->read_sequence = write_seq - topic->message_count + 1;
                }
            }

            size_t offset = topic->memory_offset + (FAST_MODULO(handle->read_sequence, topic->message_count)) * topic->message_size;

            memcpy(buffer, g_topicsMemoryBuffer + offset, topic->message_size);
        } while (topic->write_sequence.load(std::memory_order_acquire) - handle->read_sequence > topic->message_count);

        handle->read_sequence++;

        return true;
    }
}