#ifndef _BATTERY_UTILS_H
#define _BATTERY_UTILS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Battery entry data structure
 */
typedef struct
{
    float voltage;      /** Voltage */
    uint8_t percentage; /** Percentage */
} battery_table_entry_t;

/**
 * @brief Battery config data
 */
typedef struct
{
    const battery_table_entry_t *entries;   /** Battery entries */
    uint8_t entriesCount;                   /** Battery entries count */
    float oneCellMaxVoltage;                /** One cell max voltage */
} battery_config_t;

/**
 * @brief Battery conversion data
 */
typedef struct
{
    uint8_t percentage; /** Charge state */
    uint8_t nCells;     /** Number of cells */
} battery_data_t;

/**
 * @brief Initialize battery
 *
 * @param config Battery config
 * @param entries Entries in decreasing order for a single cell
 * @param entriesCount Count of entries. Must be at least 2!
 */
void battery_init(battery_config_t *config, const battery_table_entry_t *entries, uint8_t entriesCount);

/**
 * @brief Convert voltage to percent
 *
 * @param config Battery config
 * @param voltage Voltage
 * @param data Data to set
 * @return Percent
 */
void battery_convert(const battery_config_t *config, float voltage, battery_data_t *data);

#ifdef __cplusplus
}
#endif

#endif