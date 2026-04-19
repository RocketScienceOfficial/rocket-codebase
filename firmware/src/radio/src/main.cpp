#include "config.h"
#include <pico/stdlib.h>
#include <hardware/spi.h>
#include <hardware/watchdog.h>
#include <RadioLib.h>
#include <hal/RPiPico/PicoHal.h>
#include <datalink.h>

#ifndef NDEBUG
#define LOG_INFO(msg, ...) printf("[INFO]  " msg "\n", ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) printf("[ERROR]  " msg "\n", ##__VA_ARGS__)
#else
#define LOG_INFO(msg, ...) ((void)0)
#define LOG_ERROR(msg, ...) ((void)0)
#endif

static PicoHal s_HAL(SPI_INST, SPI_PIN_MISO, SPI_PIN_MOSI, SPI_PIN_SCK);
static Module s_Module(&s_HAL, LORA_PIN_CS, LORA_PIN_DIO1, LORA_PIN_RESET, LORA_PIN_BUSY);
static SX1268 s_Radio(&s_Module);
static volatile bool s_RadioOpDoneFlag;
static bool s_Transmitting;
static bool s_DisableNextTransmit;

static void _init(void);
static void _read_uart(void);
static void _handle_radio(void);
static void _process_new_uart_frame(const datalink_message_t *msg);
static void _send_frame_uart(const datalink_message_t *msg);
static void _set_radio_flag(void);

int main()
{
    _init();

    while (true)
    {
        _read_uart();
        _handle_radio();
    }

    return 0;
}

static void _init(void)
{
    stdio_init_all();

    uart_init(UART_INST, UART_BAUDRATE);

    gpio_init(UART_PIN_TX);
    gpio_set_dir(UART_PIN_TX, true);
    gpio_set_function(UART_PIN_TX, GPIO_FUNC_UART);

    gpio_init(UART_PIN_RX);
    gpio_set_dir(UART_PIN_RX, true);
    gpio_set_function(UART_PIN_RX, GPIO_FUNC_UART);

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

static void _read_uart(void)
{
    static uint8_t recvBuffer[512];
    static int recvLen = 0;

    while (uart_is_readable(UART_INST))
    {
        uint8_t byte;
        uart_read_blocking(UART_INST, &byte, 1);

        if (recvLen >= sizeof(recvBuffer))
        {
            recvLen = 0;
        }

        recvBuffer[recvLen++] = byte;

        if (byte == 0x00)
        {
            datalink_message_t msg;

            if (datalink_deserialize_message_serial(&msg, recvBuffer, recvLen) == DATALINK_OK)
            {
                _process_new_uart_frame(&msg);
            }
            else
            {
                LOG_ERROR("Couldn't deserialize UART frame!");
            }

            recvLen = 0;
        }
    }
}

static void _handle_radio(void)
{
    static uint8_t receiveBuffer[512];

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

        watchdog_disable();

        LOG_INFO("Finished transmission!");

        if (s_DisableNextTransmit)
        {
            gpio_put(LORA_PIN_TXEN, 0);
            gpio_put(LORA_PIN_RXEN, 1);

            s_Transmitting = false;
            s_DisableNextTransmit = false;

            s_Radio.startReceive();

            LOG_INFO("Started receiving...");
        }

        datalink_message_t msg;
        datalink_pack_radio_module_tx_done(&msg);

        _send_frame_uart(&msg);
    }
    else
    {
        if (!s_Radio.checkIrq(RADIOLIB_IRQ_RX_DONE))
        {
            return;
        }

        size_t packetLength = s_Radio.getPacketLength();

        if (packetLength > 0 && packetLength <= sizeof(receiveBuffer))
        {
            LOG_INFO("Received %d bytes from radio!", packetLength);

            s_Radio.readData(receiveBuffer, packetLength);

            datalink_message_t msg;
            uint8_t seq, srcId, destId;

            if (datalink_deserialize_message_radio(&msg, &seq, &srcId, &destId, receiveBuffer, packetLength) == DATALINK_ERROR)
            {
                LOG_ERROR("Couldn't deserialize radio frame!");

                return;
            }

            if (srcId != GCS_ID || destId != DEVICE_ID)
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

            _send_frame_uart(&msg);
        }
    }
}

static void _process_new_uart_frame(const datalink_message_t *msg)
{
    LOG_INFO("Processing next UART frame with message id '%d' and length '%d'", msg->msg_id, msg->len);

    if (msg->msg_id == DATALINK_MESSAGE_ID_TELEMETRY_DATA_OBC)
    {
        static uint8_t sequence = 0;
        static uint8_t buffer[512];

        int len = sizeof(buffer);

        if (datalink_serialize_message_radio(msg, sequence, DEVICE_ID, GCS_ID, buffer, &len) == DATALINK_OK)
        {
            if (!s_Transmitting)
            {
                s_Transmitting = true;

                gpio_put(LORA_PIN_TXEN, 1);
                gpio_put(LORA_PIN_RXEN, 0);
            }

            telemetry_data_obc data;

            if (datalink_unpack_telemetry_data_obc(&data, msg) == DATALINK_ERROR)
            {
                LOG_ERROR("Couldn't unpack telemetry data!");
            }

            if (data.sendResponse == 1)
            {
                s_DisableNextTransmit = true;
            }

            watchdog_enable(LORA_WATCHDOG_TIMEOUT_MS, true);

            s_Radio.startTransmit(buffer, len);

            LOG_INFO("Started transmitting %d bytes through Radio!", len);
        }
        else
        {
            LOG_ERROR("Couldn't serialize radio frame!");
        }

        sequence = sequence == 255 ? 0 : sequence + 1;
    }
}

static void _send_frame_uart(const datalink_message_t *msg)
{
    static uint8_t buffer[512];
    int len = sizeof(buffer);

    if (datalink_serialize_message_serial(msg, buffer, &len) == DATALINK_OK)
    {
        uart_write_blocking(UART_INST, buffer, len);

        LOG_INFO("Sent %d bytes to UART!", len);
    }
    else
    {
        LOG_ERROR("Couldn't serialize UART frame!");
    }
}

static void _set_radio_flag(void)
{
    s_RadioOpDoneFlag = true;
}