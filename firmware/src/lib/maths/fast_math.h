#ifndef _FAST_MATH_H
#define _FAST_MATH_H

#define FAST_MODULO(x, m) ((x) & ((m) - 1))

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Fast inverse square root from Quake III. See: https://en.wikipedia.org/wiki/Fast_inverse_square_root
 *
 * @param x Number to calculate the inverse square root of
 * @return Inverse square root of x
 */
float fast_inv_sqrt(float x);

#ifdef __cplusplus
}
#endif

#endif