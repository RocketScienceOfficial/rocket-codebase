#include "hal/adc_driver.h"
#include "adc_utils_esp32.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "hal/adc_types.h"

static adc_oneshot_unit_handle_t g_adc_handles[2] = {NULL, NULL};
static adc_cali_handle_t g_cali_handles[20] = {NULL};

static void channel_to_unit_and_channel(hal_adc_channel_t channel, adc_unit_t *unit, adc_channel_t *out_channel)
{
    *unit = (HAL_ADC_ESP32_DECODE_UNIT(channel) == 1) ? ADC_UNIT_1 : ADC_UNIT_2;
    *out_channel = (adc_channel_t)HAL_ADC_ESP32_DECODE_CHANNEL(channel);
}

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

void hal_adc_init_channel(hal_adc_channel_t channel)
{
    adc_unit_t unit;
    adc_channel_t out_channel;

    channel_to_unit_and_channel(channel, &unit, &out_channel);

    if (g_adc_handles[unit] == NULL)
    {
        return;
    }

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    adc_oneshot_config_channel(g_adc_handles[unit], out_channel, &config);

#if CONFIG_IDF_TARGET_ESP32
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = unit,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    adc_cali_create_scheme_line_fitting(&cali_config, &g_cali_handles[channel]);
#else
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = unit,
        .chan = out_channel,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    adc_cali_create_scheme_curve_fitting(&cali_config, &g_cali_handles[channel]);
#endif
}

float hal_adc_channel_read_voltage(hal_adc_channel_t channel)
{
    adc_unit_t unit;
    adc_channel_t out_channel;

    channel_to_unit_and_channel(channel, &unit, &out_channel);

    if (g_adc_handles[unit] == NULL)
    {
        return 0.0f;
    }

    int raw_val;

    if (adc_oneshot_read(g_adc_handles[unit], out_channel, &raw_val) == ESP_OK)
    {
        int voltage_mv = 0;

        if (g_cali_handles[channel] != NULL)
        {
            adc_cali_raw_to_voltage(g_cali_handles[channel], raw_val, &voltage_mv);

            return (float)voltage_mv / 1000.0f;
        }
    }

    return 0.0f;
}
