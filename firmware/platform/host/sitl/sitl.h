#ifndef _SITL_H
#define _SITL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes thread to the SITL with "god mode".
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