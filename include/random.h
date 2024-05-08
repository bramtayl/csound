#pragma once

#include "csound.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Simple linear congruential random number generator:
 *   (*seedVal) = (*seedVal) * 742938285 % 2147483647
 * the initial value of *seedVal must be in the range 1 to 2147483646.
 * Returns the next number from the pseudo-random sequence,
 * in the range 1 to 2147483646.
 */
PUBLIC int csoundRand31(int *seedVal);

/**
 * Initialise Mersenne Twister (MT19937) random number generator,
 * using 'keyLength' unsigned 32 bit values from 'initKey' as seed.
 * If the array is NULL, the length parameter is used for seeding.
 */
PUBLIC void csoundSeedRandMT(CsoundRandMTState *p, const uint32_t *initKey,
                             uint32_t keyLength);

/**
 * Returns next random number from MT19937 generator.
 * The PRNG must be initialised first by calling csoundSeedRandMT().
 */
PUBLIC uint32_t csoundRandMT(CsoundRandMTState *p);

#ifdef __cplusplus
}
#endif