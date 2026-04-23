#include "hal/i2c_driver.h"
#include "driver/i2c_master.h"
#include "esp_err.h"

#define MAX_I2C_BUSES 2
#define MAX_I2C_DEVICES 10

typedef struct
{
    bool in_use;
    uint8_t bus;
    uint8_t address;
    i2c_master_dev_handle_t handle;
} i2c_device_node_t;

static i2c_master_bus_handle_t g_bus_handles[MAX_I2C_BUSES] = {NULL};
static uint32_t g_bus_baudrates[MAX_I2C_BUSES] = {0};
static i2c_device_node_t s_device_registry[MAX_I2C_DEVICES] = {0};

void hal_i2c_init(uint8_t bus, uint8_t sda, uint8_t scl, uint32_t baudrate)
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = bus,
        .sda_io_num = sda,
        .scl_io_num = scl,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    if (i2c_new_master_bus(&bus_config, &g_bus_handles[bus]) == ESP_OK)
    {
        g_bus_baudrates[bus] = baudrate;
    }
}

static i2c_master_dev_handle_t _get_device_handle(uint8_t bus, uint8_t address)
{
    if (bus >= MAX_I2C_BUSES || g_bus_handles[bus] == NULL)
    {
        return NULL;
    }

    for (int i = 0; i < MAX_I2C_DEVICES; i++)
    {
        if (s_device_registry[i].in_use && s_device_registry[i].bus == bus && s_device_registry[i].address == address)
        {
            return s_device_registry[i].handle;
        }
    }

    int free_slot = -1;
    for (int i = 0; i < MAX_I2C_DEVICES; i++)
    {
        if (!s_device_registry[i].in_use)
        {
            free_slot = i;
            break;
        }
    }

    if (free_slot == -1)
    {
        return NULL;
    }

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = address,
        .scl_speed_hz = g_bus_baudrates[bus],
    };

    if (i2c_master_bus_add_device(g_bus_handles[bus], &dev_cfg, &s_device_registry[free_slot].handle) == ESP_OK)
    {
        s_device_registry[free_slot].in_use = true;
        s_device_registry[free_slot].bus = bus;
        s_device_registry[free_slot].address = address;
        
        return s_device_registry[free_slot].handle;
    }

    return NULL;
}

bool hal_i2c_transfer(uint8_t bus, uint8_t address, const uint8_t *tx_buffer, size_t tx_size, uint8_t *rx_buffer, size_t rx_size)
{
    i2c_master_dev_handle_t dev = _get_device_handle(bus, address);

    if (!dev)
    {
        return false;
    }

    esp_err_t err = ESP_FAIL;
    int timeout_ms = -1;

    if (tx_size > 0 && rx_size > 0)
    {
        err = i2c_master_transmit_receive(dev, tx_buffer, tx_size, rx_buffer, rx_size, timeout_ms);
    }
    else if (tx_size > 0)
    {
        err = i2c_master_transmit(dev, tx_buffer, tx_size, timeout_ms);
    }
    else if (rx_size > 0)
    {
        err = i2c_master_receive(dev, rx_buffer, rx_size, timeout_ms);
    }
    else
    {
        err = i2c_master_probe(g_bus_handles[bus], address, timeout_ms);
    }

    return err == ESP_OK;
}