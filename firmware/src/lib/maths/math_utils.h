#ifndef _MATH_UTILS_H
#define _MATH_UTILS_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Clamps float type value between min and max
 * 
 * @param x Value
 * @param min Minimum value
 * @param max Maximum value
 * @return Clamped value
*/
float clamp_value(float x, float min, float max);

/**
 * @brief Check if value is approximately equal desired value
 * 
 * @param val Value to be checked
 * @param des Desired value
 * @param eps Accepted error
 * @return true if value is approximately equal desired value
*/
bool value_approx_eql(float val, float des, float eps);

/**
 * @brief Exponential smoothing of value
 * 
 * @param x1 New value
 * @param x0 Previous value
 * @param a Smoothing factor (0-1)
 * @return Smoothed value
 */
float exp_smoothing(float x1, float x0, float a);

#ifdef __cplusplus
}
#endif

#endif