#include "battery_interp_model.h"
#include <lib/maths/math_utils.h>
#include <lib/debug/sys_assert.h>
#include <math.h>

void battery_init(battery_config_t *config, const battery_table_entry_t *entries, uint8_t entriesCount)
{
    SYS_ASSERT(config != NULL);
    SYS_ASSERT(entries != NULL);
    SYS_ASSERT(entriesCount >= 2);

    config->entries = entries;
    config->entriesCount = entriesCount;
    config->oneCellMaxVoltage = entries[0].voltage;
}

void battery_convert(const battery_config_t *config, float voltage, battery_data_t *data)
{
    SYS_ASSERT(config != NULL);
    SYS_ASSERT(data != NULL);
    SYS_ASSERT(config->entries != NULL);
    SYS_ASSERT(config->entriesCount >= 2);
    SYS_ASSERT(config->oneCellMaxVoltage > 0.0f);

    data->nCells = (uint8_t)ceilf(voltage / config->oneCellMaxVoltage);
    data->percentage = 0;

    if (data->nCells > 0)
    {
        float oneCellVoltage = voltage / data->nCells;

        for (uint8_t i = 1; i < config->entriesCount; i++)
        {
            if (oneCellVoltage >= config->entries[i].voltage && oneCellVoltage <= config->entries[i - 1].voltage)
            {
                float dv = config->entries[i - 1].voltage - config->entries[i].voltage;
                float dp = (float)config->entries[i - 1].percentage - (float)config->entries[i].percentage;

                SYS_ASSERT(dv != 0.0f);

                float t = (oneCellVoltage - config->entries[i].voltage) / dv;
                float percentage = (float)config->entries[i].percentage + dp * t;

                data->percentage = (uint8_t)clamp_value(percentage, 0.0f, 100.0f);

                break;
            }
        }
    }
}
