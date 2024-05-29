#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "csound.h"

/**
 * Compute in-place real FFT, allowing non power of two FFT sizes.
 *
 * buf:     array of FFTsize + 2 MYFLT values; output is in interleaved
 *          real/imaginary format (note: the real part of the Nyquist
 *          frequency is stored in buf[FFTsize], and not in buf[1]).
 * FFTsize: FFT length in samples; not required to be an integer power of two,
 *          but should be even and not have too many factors.
 */
PUBLIC void csoundRealFFTnp2(CSOUND *csound, MYFLT *buf, int32_t FFTsize);

/**
 * Compute in-place inverse real FFT, allowing non power of two FFT sizes.
 * The output does not need to be scaled.
 *
 * buf:     array of FFTsize + 2 MYFLT values, in interleaved real/imaginary
 *          format (note: the real part of the Nyquist frequency is stored
 *          in buf[FFTsize], and not in buf[1]).
 * FFTsize: FFT length in samples; not required to be an integer power of two,
 *          but should be even and not have too many factors.
 */
PUBLIC void csoundInverseRealFFTnp2(CSOUND *csound, MYFLT *buf,
                                    int32_t FFTsize);

#ifdef __cplusplus
}
#endif