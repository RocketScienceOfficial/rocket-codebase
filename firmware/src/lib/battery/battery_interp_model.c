#include "battery_interp_model.h"
#include <lib/maths/math_utils.h>
#include <lib/debug/obc_assert.h>
#include <math.h>

void battery_init(battery_config_t *config, const battery_table_entry_t *entries, uint8_t entriesCount)
{
    OBC_ASSERT(config != NULL);
    OBC_ASSERT(entries != NULL);
    OBC_ASSERT(entriesCount >= 2);

    config->entries = entries;
    config->entriesCount = entriesCount;
    config->oneCellMaxVoltage = entries[0].voltage;
}

void battery_convert(const battery_config_t *config, float voltage, battery_data_t *data)
{
    OBC_ASSERT(config != NULL);
    OBC_ASSERT(data != NULL);
    OBC_ASSERT(config->entries != NULL);
    OBC_ASSERT(config->entriesCount >= 2);
    OBC_ASSERT(config->oneCellMaxVoltage > 0.0f);

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

                OBC_ASSERT(dv != 0.0f);

                float t = (oneCellVoltage - config->entries[i].voltage) / dv;
                float percentage = (float)config->entries[i].percentage + dp * t;

                data->percentage = (uint8_t)clamp_value(percentage, 0.0f, 100.0f);

                break;
            }
        }
    }
}
