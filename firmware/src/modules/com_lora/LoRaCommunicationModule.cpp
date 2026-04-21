#include "LoRaCommunicationModule.h"
#include "modules/common/ModuleLogger.h"
#include <board_config.h>
#include <RadioLib.h>
#include <hal/gpio_driver.h>
#include <lib/drivers/radio/RadioLibHALPort.h>
#include <lib/debug/sys_assert.h>

static RadioLibHALPort g_hal(CFG_LORA_SPI, CFG_LORA_SPI_MISO_PIN, CFG_LORA_SPI_MOSI_PIN, CFG_LORA_SPI_SCK_PIN);
static Module g_module(&g_hal, CFG_LORA_PIN_CS, CFG_LORA_PIN_DIO1, CFG_LORA_PIN_RESET, CFG_LORA_PIN_BUSY);
static SX1268 g_radio(&g_module);
static volatile bool g_radioOpDoneFlag;
static bool g_radioInitialized = false;

static void _set_radio_flag(void)
{
    g_radioOpDoneFlag = true;
}

void LoRaCommunicationModule::init()
{
    SYS_ASSERT_MSG(g_radioInitialized == false, "LoRa radio already initialized!");

    hal_gpio_init_pin(CFG_LORA_PIN_TXEN, GPIO_OUTPUT);
    hal_gpio_set_pin_state(CFG_LORA_PIN_TXEN, GPIO_LOW);

    hal_gpio_init_pin(CFG_LORA_PIN_RXEN, GPIO_OUTPUT);
    hal_gpio_set_pin_state(CFG_LORA_PIN_RXEN, GPIO_LOW);

    int state = g_radio.begin(CFG_LORA_FREQ, CFG_LORA_BANDWIDTH, CFG_LORA_SF, 5, 0x12, CFG_LORA_TX_POWER, 8, 3.3f, false);

    SYS_ASSERT_MSG(state == RADIOLIB_ERR_NONE, "Failed to initialize LoRa radio! Code: %d", state);

    g_radio.setDio1Action(_set_radio_flag);

    LOG_INFO("LoRa radio initialized successfully!");

    g_radioInitialized = true;
}

void LoRaCommunicationModule::run()
{
    checkIncomingMessages();
    checkRadio();
}

void LoRaCommunicationModule::checkIncomingMessages()
{
    if (m_Subscriber.poll())
    {
        const auto &msg = m_Subscriber.get();
        int len = sizeof(m_TransmitBuffer);

        if (datalink_serialize_message_radio(&msg, m_Sequence, CFG_RADIO_SRC_ID, CFG_RADIO_DST_ID, m_TransmitBuffer, &len) == DATALINK_OK)
        {
            setTX();

            telemetry_data_obc data;
            int unpackResult = datalink_unpack_telemetry_data_obc(&data, &msg);

            SYS_ASSERT_MSG(unpackResult == DATALINK_OK, "Couldn't unpack telemetry data! Error code: %d", unpackResult);

            if (data.sendResponse == 1)
            {
                m_DisableNextTransmit = true;

                LOG_DEBUG("Disabling TX after this transmission");
            }

            g_radio.startTransmit(m_TransmitBuffer, len);

            m_Sequence = m_Sequence == 255 ? 0 : m_Sequence + 1;

            LOG_INFO("Started transmitting %d bytes through Radio!", len);
        }
        else
        {
            LOG_ERROR("Couldn't serialize radio frame!");
        }
    }
}

void LoRaCommunicationModule::checkRadio()
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

void LoRaCommunicationModule::handleTX()
{
    g_radio.finishTransmit();

    LOG_INFO("Finished transmission!");

    if (m_DisableNextTransmit)
    {
        m_DisableNextTransmit = false;

        setRX();
    }

    datalink_message_t msg;
    datalink_pack_radio_module_tx_done(&msg);
    m_Publisher.publish(msg);
}

void LoRaCommunicationModule::handleRX()
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

        if (srcId != CFG_RADIO_DST_ID || destId != CFG_RADIO_SRC_ID)
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

        m_Publisher.publish(msg);
    }
}

void LoRaCommunicationModule::setTX()
{
    if (!m_Transmitting)
    {
        hal_gpio_set_pin_state(CFG_LORA_PIN_TXEN, GPIO_HIGH);
        hal_gpio_set_pin_state(CFG_LORA_PIN_RXEN, GPIO_LOW);

        m_Transmitting = true;

        LOG_INFO("Started transmitting mode...");
    }
}

void LoRaCommunicationModule::setRX()
{
    if (m_Transmitting)
    {
        hal_gpio_set_pin_state(CFG_LORA_PIN_TXEN, GPIO_LOW);
        hal_gpio_set_pin_state(CFG_LORA_PIN_RXEN, GPIO_HIGH);

        g_radio.startReceive();

        m_Transmitting = false;

        LOG_INFO("Started receiving mode...");
    }
}