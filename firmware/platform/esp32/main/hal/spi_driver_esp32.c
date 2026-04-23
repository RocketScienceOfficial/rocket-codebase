#include "hal/spi_driver.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

static spi_device_handle_t g_spi_handles[SOC_SPI_PERIPH_NUM];

void hal_spi_init(uint8_t bus, uint8_t miso, uint8_t mosi, uint8_t sck, uint32_t baudrate)
{
    spi_bus_config_t buscfg = {};
    buscfg.miso_io_num = miso;
    buscfg.mosi_io_num = mosi;
    buscfg.sclk_io_num = sck;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = 0;

    spi_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz = baudrate;
    devcfg.mode = 0;
    devcfg.spics_io_num = -1;
    devcfg.queue_size = 1;

    spi_bus_initialize((spi_host_device_t)bus, &buscfg, SPI_DMA_CH_AUTO);
    spi_bus_add_device((spi_host_device_t)bus, &devcfg, &g_spi_handles[bus]);
}

bool hal_spi_transfer(uint8_t bus, const uint8_t *txData, uint8_t *rxData, size_t size)
{
    spi_transaction_t t = {};
    t.length = size * 8;
    t.tx_buffer = txData;
    t.rx_buffer = rxData;

    return spi_device_transmit(g_spi_handles[bus], &t) == ESP_OK;
}