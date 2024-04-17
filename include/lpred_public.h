#pragma once

#include "csound.h"

#ifdef __cplusplus
extern "C" {
#endif /*  __cplusplus */

/**
 * Compute autocorrelation function
 * r: autocorrelation output array (size N)
 * s: input signal
 * N: signal size
 * b: fft buffer (if NULL then time-domain algorithm is used)
 * NF: fft size, power-of-two (NF >= N*2-1)
 * returns: autocorrelation r
 */
PUBLIC MYFLT *csoundAutoCorrelation(CSOUND *csound, MYFLT *r, MYFLT *s,
                                    int size, MYFLT *b, int NF);

/**
 * Linear prediction setup
 *
 * N: autocorrelation size
 * M: filter order
 *
 * returns: opaque LP structure to use with linear prediction function
 */
PUBLIC void *csoundLPsetup(CSOUND *csound, int N, int M);

/**
 * Linear prediction setup deallocation
 *
 * param: LP setup object
 *
 */

PUBLIC void csoundLPfree(CSOUND *csound, void *param);

/**
 * Compute linear prediction coefficients
 *
 * x: input signal
 * param: LP setup object
 *
 * returns: array of size M+1 with error E and coefficients 1-M
 * output format is [E,c1,c2,...,cm] OR NULL if a memory problem occured
 * NB: c0 is always 1
 */
PUBLIC MYFLT *csoundLPred(CSOUND *csound, void *p, MYFLT *x);

/**
 * Compute all-pole coefficients and linear prediction error
 * from cepstrum coefficients
 *
 * b: array of size M+1
 * c: array of size N with cepstrum coeffs
 *
 * M: all-pole filter order
 * N: cepstrum size
 *
 * returns: M+1 size array with all-pole coefficients 1-M and
 * E in place of coefficient 0 [E,c1,...,cM]
 * NB: cepstrum is expected to be computed from power spectrum
 */
PUBLIC MYFLT *csoundLPCeps(CSOUND *csound, MYFLT *c, MYFLT *b, int N, int M);

/**
 * Compute cepstrum coefficients from all-pole coefficients
 * and linear prediction error
 *
 * c: array of size N
 * b: array of size M+1 with M all-pole coefficients
 *   and E in place of coefficient 0 [E,c1,...,cM]
 * N: size of cepstrum array output
 * M: all-pole filter order
 *
 * returns: array with N cepstrum coefficients
 * NB: cepstrum is computed from power spectrum
 */
PUBLIC MYFLT *csoundCepsLP(CSOUND *csound, MYFLT *b, MYFLT *c, int M, int N);

/**
 * Returns the computed RMS from LP object
 */
PUBLIC MYFLT csoundLPrms(CSOUND *csound, void *parm);

#ifdef __cplusplus
}
#endif /* __cplusplus */