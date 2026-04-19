#include "hal/uart_driver.h"
#include "hal/gpio_driver.h"
#include "hardware/uart.h"
#include "hardware/structs/uart.h"
#include "hardware/dma.h"
#include "pico/stdlib.h"
#include <string.h>

#define RING_SIZE 256
#define RING_OVERFLOW_SAFETY_MARGIN 32
#define RING_BITS 8

static_assert((RING_SIZE & (RING_SIZE - 1)) == 0, "RING_SIZE must be a power of 2");

typedef struct
{
    uint8_t dma_buffer[RING_SIZE] __attribute__((aligned(RING_SIZE)));
    int dma_chan_rx;
    int dma_chan_tx;
    uint32_t read_seq;
    uint32_t absolute_bytes_written;
    uart_inst_t *uart;
    uart_hw_t *uart_hw;
} uart_context_t;

static uart_context_t g_uart0_ctx = {
    .uart = uart0,
    .uart_hw = uart0_hw,
};
static uart_context_t g_uart1_ctx = {
    .uart = uart1,
    .uart_hw = uart1_hw,
};

static uart_context_t *_get_ctx(uint8_t uart)
{
    return (uart == 0) ? &g_uart0_ctx : &g_uart1_ctx;
}

static void _dma_setup_rx(uart_context_t *ctx)
{
    ctx->dma_chan_rx = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(ctx->dma_chan_rx);

    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, true);
    channel_config_set_ring(&c, true, RING_BITS);
    channel_config_set_dreq(&c, uart_get_dreq(ctx->uart, false));

    dma_channel_configure(ctx->dma_chan_rx, &c, ctx->dma_buffer, ctx->uart_hw, 0xFFFFFFFF, true);
}

static void _dma_setup_tx(uart_context_t *ctx)
{
    ctx->dma_chan_tx = dma_claim_unused_channel(true);
    dma_channel_config c_tx = dma_channel_get_default_config(ctx->dma_chan_tx);

    channel_config_set_transfer_data_size(&c_tx, DMA_SIZE_8);
    channel_config_set_read_increment(&c_tx, true);
    channel_config_set_write_increment(&c_tx, false);
    channel_config_set_dreq(&c_tx, uart_get_dreq(ctx->uart, true));

    dma_channel_configure(ctx->dma_chan_tx, &c_tx, &ctx->uart_hw->dr, NULL, 0, false);
}

void hal_uart_init(uint8_t bus, uint8_t rx, uint8_t tx, uint32_t baudrate)
{
    uart_context_t *ctx = _get_ctx(bus);

    uart_init(ctx->uart, baudrate);

    hal_gpio_set_pin_function(rx, GPIO_FUNCTION_UART);
    hal_gpio_set_pin_function(tx, GPIO_FUNCTION_UART);

    uart_set_fifo_enabled(ctx->uart, true);

    _dma_setup_rx(ctx);
    _dma_setup_tx(ctx);
}

bool hal_uart_is_writable(uint8_t bus)
{
    uart_context_t *ctx = _get_ctx(bus);

    return !dma_channel_is_busy(ctx->dma_chan_tx);
}

void hal_uart_write(uint8_t bus, const uint8_t *data, size_t size)
{
    uart_context_t *ctx = _get_ctx(bus);

    dma_channel_transfer_from_buffer_now(ctx->dma_chan_tx, data, size);
}

bool hal_uart_fifo_available(uint8_t bus)
{
    uart_context_t *ctx = _get_ctx(bus);

    uint32_t current_transfer_count = dma_hw->ch[ctx->dma_chan_rx].transfer_count;
    ctx->absolute_bytes_written = 0xFFFFFFFF - current_transfer_count;
    uint32_t unread_bytes = ctx->absolute_bytes_written - ctx->read_seq;

    if (unread_bytes > RING_SIZE)
    {
        ctx->read_seq = ctx->absolute_bytes_written - RING_SIZE + RING_OVERFLOW_SAFETY_MARGIN;
    }

    return unread_bytes > 0;
}

size_t hal_uart_read_fifo(uint8_t bus, uint8_t *buffer, size_t bufSize)
{
    uart_context_t *ctx = _get_ctx(bus);
    
    size_t read_index = ctx->read_seq % RING_SIZE;
    size_t bytes_to_end = RING_SIZE - 1 - read_index;

    size_t size = ctx->absolute_bytes_written - ctx->read_seq;
    size = (size > bufSize) ? bufSize : size;

    if (size <= bytes_to_end)
    {
        memcpy(buffer, &ctx->dma_buffer[read_index], size);
    }
    else
    {
        memcpy(buffer, &ctx->dma_buffer[read_index], bytes_to_end);
        size_t remaining = size - bytes_to_end;
        memcpy(buffer + bytes_to_end, &ctx->dma_buffer[0], remaining);
    }

    ctx->read_seq += size;

    return size;
}