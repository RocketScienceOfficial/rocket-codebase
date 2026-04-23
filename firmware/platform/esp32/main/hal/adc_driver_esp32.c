#include "hal/adc_driver.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "hal/adc_types.h"

static adc_oneshot_unit_handle_t g_adc_handles[2] = {NULL, NULL};
static adc_cali_handle_t g_cali_handles[GPIO_NUM_MAX] = {NULL};

void hal_adc_init_all(void)
{
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
    };
    adc_oneshot_new_unit(&init_config1, &g_adc_handles[ADC_UNIT_1]);

    adc_oneshot_unit_init_cfg_t init_config2 = {
        .unit_id = ADC_UNIT_2,
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
    };
    adc_oneshot_new_unit(&init_config2, &g_adc_handles[ADC_UNIT_2]);
}

void hal_adc_init_pin(uint8_t pin)
{
    adc_unit_t unit;
    adc_channel_t channel;

    if (adc_oneshot_io_to_channel(pin, &unit, &channel) != ESP_OK)
    {
        return;
    }

    if (g_adc_handles[unit] == NULL)
    {
        return;
    }

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    adc_oneshot_config_channel(g_adc_handles[unit], channel, &config);

#if CONFIG_IDF_TARGET_ESP32
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = unit,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    adc_cali_create_scheme_line_fitting(&cali_config, &g_cali_handles[pin]);
#else
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = unit,
        .chan = channel,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    adc_cali_create_scheme_curve_fitting(&cali_config, &g_cali_handles[pin]);
#endif
}

float hal_adc_read_voltage(uint8_t pin)
{
    adc_unit_t unit;
    adc_channel_t channel;

    if (adc_oneshot_io_to_channel(pin, &unit, &channel) != ESP_OK)
    {
        return 0.0f;
    }

    if (g_adc_handles[unit] == NULL)
    {
        return 0.0f;
    }

    int raw_val;

    if (adc_oneshot_read(g_adc_handles[unit], channel, &raw_val) == ESP_OK)
    {
        int voltage_mv = 0;

        if (g_cali_handles[pin] != NULL)
        {
            adc_cali_raw_to_voltage(g_cali_handles[pin], raw_val, &voltage_mv);

            return (float)voltage_mv / 1000.0f;
        }
    }

    return 0.0f;
}