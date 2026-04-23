#include "hal/uart_driver.h"
#include "driver/uart.h"

static uart_port_t g_uart_ports[UART_NUM_MAX];

void hal_uart_init(uint8_t bus, uint8_t rx, uint8_t tx, uint32_t baudrate)
{
    uart_config_t cfg = {
        .baud_rate = baudrate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    uart_port_t port = (uart_port_t)bus;
    g_uart_ports[bus] = port;

    uart_driver_install(port, 1024, 1024, 0, NULL, 0);
    uart_param_config(port, &cfg);
    uart_set_pin(port, tx, rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

bool hal_uart_is_writable(uint8_t bus)
{
    uart_port_t port = g_uart_ports[bus];
    size_t free_size;

    uart_get_tx_buffer_free_size(port, &free_size);

    return free_size > 0;
}

void hal_uart_write(uint8_t bus, const uint8_t *data, size_t size)
{
    uart_port_t port = g_uart_ports[bus];
    uart_write_bytes(port, (const char *)data, size);
}

bool hal_uart_fifo_available(uint8_t bus)
{
    uart_port_t port = g_uart_ports[bus];
    size_t len;

    uart_get_buffered_data_len(port, &len);

    return len > 0;
}

size_t hal_uart_read_fifo(uint8_t bus, uint8_t *buffer, size_t bufSize)
{
    uart_port_t port = g_uart_ports[bus];
    size_t available;

    uart_get_buffered_data_len(port, &available);

    size_t to_read = available < bufSize ? available : bufSize;

    if (to_read == 0)
    {
        return 0;
    }

    int len = uart_read_bytes(port, buffer, to_read, 0);

    return len > 0 ? (size_t)len : 0;
}