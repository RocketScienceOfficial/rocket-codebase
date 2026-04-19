#include "com_lora.h"
#include "config.h"
#include "logger.h"
#include <pico/stdlib.h>
#include <hardware/spi.h>
#include <RadioLib.h>
#include <hal/RPiPico/PicoHal.h>

static PicoHal s_HAL(SPI_INST, SPI_PIN_MISO, SPI_PIN_MOSI, SPI_PIN_SCK);
static Module s_Module(&s_HAL, LORA_PIN_CS, LORA_PIN_DIO1, LORA_PIN_RESET, LORA_PIN_BUSY);
static SX1268 s_Radio(&s_Module);

static volatile bool s_RadioOpDoneFlag;
static bool s_Transmitting;

static void _set_radio_flag(void)
{
    s_RadioOpDoneFlag = true;
}

void CommunicationLoRa::init()
{
    gpio_init(LORA_PIN_TXEN);
    gpio_set_dir(LORA_PIN_TXEN, true);
    gpio_put(LORA_PIN_TXEN, 0);

    gpio_init(LORA_PIN_RXEN);
    gpio_set_dir(LORA_PIN_RXEN, true);
    gpio_put(LORA_PIN_RXEN, 0);

    LOG_INFO("Initializing SX1268...");

    int state = s_Radio.begin(LORA_FREQ, LORA_BANDWIDTH, LORA_SF, 5, 0x12, LORA_TX_POWER, 8, 3.3f, false);

    if (state != RADIOLIB_ERR_NONE)
    {
        LOG_ERROR("Failed, code %d", state);

        while (true)
        {
            tight_loop_contents();
        }
    }
    else
    {
        LOG_INFO("Initialization successful!");
    }

    s_Radio.setDio1Action(_set_radio_flag);
}

bool CommunicationLoRa::getPacket(datalink_message_t *msg)
{
    if (m_HasPacket)
    {
        *msg = m_CurrentPacket;
        m_HasPacket = false;
        
        return true;
    }

    return false;
}

void CommunicationLoRa::sendMessage(const datalink_message_t *msg)
{
    static uint8_t sequence = 0;

    uint8_t buffer[512];
    int len = sizeof(buffer);

    if (datalink_serialize_message_radio(msg, sequence, DEVICE_ID, GCS_ID, buffer, &len) == DATALINK_OK)
    {
        setTX();

        s_Radio.startTransmit(buffer, len);

        LOG_INFO("Started transmitting %d bytes through Radio!", len);
    }
    else
    {
        LOG_ERROR("Couldn't serialize radio frame!");
    }

    sequence = sequence == 255 ? 0 : sequence + 1;
}

void CommunicationLoRa::update()
{
    if (!s_RadioOpDoneFlag)
    {
        return;
    }

    s_RadioOpDoneFlag = false;

    if (s_Transmitting)
    {
        if (!s_Radio.checkIrq(RADIOLIB_IRQ_TX_DONE))
        {
            return;
        }

        s_Radio.finishTransmit();

        LOG_INFO("Finished transmission!");
    }
    else
    {
        if (!s_Radio.checkIrq(RADIOLIB_IRQ_RX_DONE))
        {
            return;
        }

        uint8_t receiveBuffer[512];
        size_t packetLength = s_Radio.getPacketLength();

        if (packetLength > 0 && packetLength <= sizeof(receiveBuffer))
        {
            LOG_INFO("Received %d bytes from radio!", packetLength);

            s_Radio.readData(receiveBuffer, packetLength);

            uint8_t seq, srcId, destId;

            if (datalink_deserialize_message_radio(&m_CurrentPacket, &seq, &srcId, &destId, receiveBuffer, packetLength) == DATALINK_ERROR)
            {
                LOG_ERROR("Couldn't deserialize radio frame!");

                return;
            }

            if (srcId != GCS_ID || destId != DEVICE_ID)
            {
                LOG_ERROR("Couldn't validate ids! (src: %d, dest: %d)", srcId, destId);

                return;
            }

            if (m_CurrentPacket.msg_id != DATALINK_MESSAGE_ID_TELEMETRY_RESPONSE)
            {
                LOG_ERROR("Invalid message id! Expected %d, got %d", DATALINK_MESSAGE_ID_TELEMETRY_RESPONSE, m_CurrentPacket.msg_id);

                return;
            }

            LOG_INFO("Successfully parsed packet! (Sequence: %d)", seq);

            m_HasPacket = true;
        }
    }
}

void CommunicationLoRa::setRX()
{
    gpio_put(LORA_PIN_TXEN, 0);
    gpio_put(LORA_PIN_RXEN, 1);

    s_Radio.startReceive();

    s_Transmitting = false;
}

void CommunicationLoRa::setTX()
{
    gpio_put(LORA_PIN_TXEN, 1);
    gpio_put(LORA_PIN_RXEN, 0);

    s_Transmitting = true;
}