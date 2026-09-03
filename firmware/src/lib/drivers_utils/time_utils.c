#include "time_utils.h"
#include <osal/systime.h>

void time_utils_delay_us_osal(uint32_t delayUs)
{
    osal_task_delay_ms((delayUs + 999) / 1000);
}