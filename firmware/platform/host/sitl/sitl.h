#ifndef _SITL_H
#define _SITL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes thread to the SITL with "god mode".
 * 
 * @warning This function is intended for use by the sim modules which do not have a rate in lockstep mode and thus do not call osal_task_delay_until(). It is safe to call it in freerun mode regarding rate.
 */
void sitl_init_godmode(void);

/**
 * @brief Advances the simulation time by one tick.
 */
void sitl_time_tick(void);

/**
 * @brief Waits for all threads to be ready.
 */
void sitl_wait_for_threads_ready(void);

/**
 * @brief Stops the SITL simulation.
 */
void sitl_stop(void);

#ifdef __cplusplus
}
#endif

#endif