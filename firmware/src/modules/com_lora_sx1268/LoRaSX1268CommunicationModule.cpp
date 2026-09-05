#include "LoRaSX1268CommunicationModule.h"
#include "modules/common/ModuleLogger.h"
#include <hal/gpio_driver.h>
#include <lib/debug/sys_assert.h>

static volatile bool g_radio_op_done_flag = false;
static bool g_radio_is_initialized = false;

static void set_radio_op_done_flag(void)
{
    g_radio_op_done_flag = true;
}

void LoRaSX1268CommunicationModule::init()
{
    // Lock other instances
    SYS_ASSERT_MSG(g_radio_is_initialized == false, "LoRa radio already initialized! / Duplicate module instance?");
    g_radio_is_initialized = true;

    // Init tx/rx enable pins
    LOG_INFO("Using TX/RX enable pins for LoRa radio...");

    hal_gpio_init_pin(m_TxenPin, HAL_GPIO_OUTPUT);
    hal_gpio_set_pin_state(m_TxenPin, HAL_GPIO_LOW);

    hal_gpio_init_pin(m_RxenPin, HAL_GPIO_OUTPUT);
    hal_gpio_set_pin_state(m_RxenPin, HAL_GPIO_LOW);

    // Setup radio
    int state = m_Radio.begin(m_RadioFrequencyMHz, m_RadioBandwidthKHz, m_RadioSpreadingFactor, 5, 0x12, m_RadioTransmitPower, 8, 3.3f, false);
    SYS_ASSERT_MSG(state == RADIOLIB_ERR_NONE, "Failed to initialize LoRa radio! Code: %d", state);
    m_Radio.setDio1Action(set_radio_op_done_flag);

    // Epilogue
    setRX();

    LOG_INFO("LoRa radio initialized successfully!");
}

void LoRaSX1268CommunicationModule::run()
{
    checkIncomingMessages();
    checkRadio();
}

void LoRaSX1268CommunicationModule::checkIncomingMessages()
{
    if (m_Subscriber.poll())
    {
        const auto &msg = m_Subscriber.get();
        int len = sizeof(m_TransmitBuffer);

        if (datalink_serialize_message_radio(&msg.msg, msg.sequence, m_srcId, m_dstId, m_TransmitBuffer, &len) == DATALINK_OK)
        {
            setTX();

            m_Radio.startTransmit(m_TransmitBuffer, len);

            LOG_INFO("Started transmitting %d bytes through Radio!", len);
        }
        else
        {
            LOG_ERROR("Couldn't serialize radio frame!");
        }
    }
}

void LoRaSX1268CommunicationModule::checkRadio()
{
    if (!g_radio_op_done_flag)
    {
        return;
    }

    g_radio_op_done_flag = false;

    if (m_Transmitting)
    {
        if (!m_Radio.checkIrq(RADIOLIB_IRQ_TX_DONE))
        {
            return;
        }

        handleTX();
    }
    else
    {
        if (!m_Radio.checkIrq(RADIOLIB_IRQ_RX_DONE))
        {
            return;
        }

        handleRX();
    }
}

void LoRaSX1268CommunicationModule::handleTX()
{
    m_Radio.finishTransmit();

    LOG_INFO("Finished transmission!");

    setRX();

    m_AckPublisher.publish({0});
}

void LoRaSX1268CommunicationModule::handleRX()
{
    size_t packetLength = m_Radio.getPacketLength();

    if (packetLength > 0 && packetLength <= sizeof(m_ReceiveBuffer))
    {
        LOG_INFO("Received %d bytes from radio!", packetLength);

        m_Radio.readData(m_ReceiveBuffer, packetLength);

        datalink_message_t msg;
        uint8_t seq, srcId, destId;

        if (datalink_deserialize_message_radio(&msg, &seq, &srcId, &destId, m_ReceiveBuffer, packetLength) == DATALINK_ERROR)
        {
            LOG_ERROR("Couldn't deserialize radio frame!");

            return;
        }

        if (srcId != m_dstId || destId != m_srcId) // Compare reversed because we are receiving a message from the other side
        {
            LOG_ERROR("Couldn't validate ids! (src: %d, dest: %d)", srcId, destId);

            return;
        }

        LOG_INFO("Successfully parsed packet! (Sequence: %d)", seq);

        m_RXPublisher.publish({
            .msg = msg,
            .rssi = (int)m_Radio.getRSSI(),
            .sequence = seq,
        });
    }
}

void LoRaSX1268CommunicationModule::setTX()
{
    hal_gpio_set_pin_state(m_TxenPin, HAL_GPIO_HIGH);
    hal_gpio_set_pin_state(m_RxenPin, HAL_GPIO_LOW);

    m_Transmitting = true;

    LOG_INFO("Started transmitting mode...");
}

void LoRaSX1268CommunicationModule::setRX()
{
    hal_gpio_set_pin_state(m_TxenPin, HAL_GPIO_LOW);
    hal_gpio_set_pin_state(m_RxenPin, HAL_GPIO_HIGH);

    m_Radio.startReceive();

    m_Transmitting = false;

    LOG_INFO("Started receiving mode...");
}