#include "LoRaGCSCommunicationModule.h"
#include "modules/common/ModuleLogger.h"
#include <board_config.h>
#include <RadioLib.h>
#include <hal/gpio_driver.h>
#include <lib/drivers/radio/RadioLibHALPort.h>
#include <lib/debug/sys_assert.h>

static RadioLibHALPort g_hal(CFG_LORA_SPI, CFG_LORA_SPI_MISO_PIN, CFG_LORA_SPI_MOSI_PIN, CFG_LORA_SPI_SCK_PIN);
static Module g_module(&g_hal, CFG_LORA_PIN_CS, CFG_LORA_PIN_DIO0, CFG_LORA_PIN_RESET);
static SX1278 g_radio(&g_module);
static volatile bool g_radioOpDoneFlag;
static bool g_radioInitialized = false;

static void _set_radio_flag(void)
{
    g_radioOpDoneFlag = true;
}

void LoRaGCSCommunicationModule::init()
{
    SYS_ASSERT_MSG(g_radioInitialized == false, "LoRa radio already initialized!");
    int state = g_radio.begin(CFG_LORA_FREQ, CFG_LORA_BANDWIDTH, CFG_LORA_SF, 5, 0x12, CFG_LORA_TX_POWER, 8);
    SYS_ASSERT_MSG(state == RADIOLIB_ERR_NONE, "Failed to initialize LoRa radio! Code: %d", state);

    g_radio.setDio0Action(_set_radio_flag, GPIO_IRQ_RISING_EDGE);

    setRX();

    LOG_INFO("LoRa radio initialized successfully!");

    g_radioInitialized = true;
}

void LoRaGCSCommunicationModule::run()
{
    checkIncomingMessages();
    checkRadio();
}

void LoRaGCSCommunicationModule::checkIncomingMessages()
{
    if (m_Subscriber.poll())
    {
        const auto &msg = m_Subscriber.get();
        int len = sizeof(m_TransmitBuffer);

        if (datalink_serialize_message_radio(&msg.msg, msg.sequence, CFG_GCS_SRC_ID, CFG_GCS_DST_ID, m_TransmitBuffer, &len) == DATALINK_OK)
        {
            setTX();

            g_radio.startTransmit(m_TransmitBuffer, len);

            LOG_INFO("Started transmitting %d bytes through Radio!", len);
        }
        else
        {
            LOG_ERROR("Couldn't serialize radio frame!");
        }
    }
}

void LoRaGCSCommunicationModule::checkRadio()
{
    if (!g_radioOpDoneFlag)
    {
        return;
    }

    g_radioOpDoneFlag = false;

    if (m_Transmitting)
    {
        if (!g_radio.checkIrq(RADIOLIB_IRQ_TX_DONE))
        {
            return;
        }

        handleTX();
    }
    else
    {
        if (!g_radio.checkIrq(RADIOLIB_IRQ_RX_DONE))
        {
            return;
        }

        handleRX();
    }
}

void LoRaGCSCommunicationModule::handleTX()
{
    g_radio.finishTransmit();

    LOG_INFO("Finished transmission!");

    setRX();
}

void LoRaGCSCommunicationModule::handleRX()
{
    size_t packetLength = g_radio.getPacketLength();

    if (packetLength > 0 && packetLength <= sizeof(m_ReceiveBuffer))
    {
        LOG_INFO("Received %d bytes from radio!", packetLength);

        g_radio.readData(m_ReceiveBuffer, packetLength);

        datalink_message_t msg;
        uint8_t seq, srcId, destId;

        if (datalink_deserialize_message_radio(&msg, &seq, &srcId, &destId, m_ReceiveBuffer, packetLength) == DATALINK_ERROR)
        {
            LOG_ERROR("Couldn't deserialize radio frame!");

            return;
        }

        if (srcId != CFG_GCS_DST_ID || destId != CFG_GCS_SRC_ID)
        {
            LOG_ERROR("Couldn't validate ids! (src: %d, dest: %d)", srcId, destId);

            return;
        }

        if (msg.msg_id != DATALINK_MESSAGE_ID_TELEMETRY_RESPONSE)
        {
            LOG_ERROR("Invalid message id! Expected %d, got %d", DATALINK_MESSAGE_ID_TELEMETRY_RESPONSE, msg.msg_id);

            return;
        }

        LOG_INFO("Successfully parsed packet! (Sequence: %d)", seq);

        m_Publisher.publish({
            .msg = msg,
            .rssi = (int)g_radio.getRSSI(),
            .sequence = seq,
        });
    }
}

void LoRaGCSCommunicationModule::setTX()
{
    if (!m_Transmitting)
    {
        m_Transmitting = true;

        LOG_INFO("Started transmitting mode...");
    }
}

void LoRaGCSCommunicationModule::setRX()
{
    if (m_Transmitting)
    {
        g_radio.startReceive();

        m_Transmitting = false;

        LOG_INFO("Started receiving mode...");
    }
}