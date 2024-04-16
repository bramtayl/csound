#include "ugens4_public.h"

#include "ugens4.h"

MYFLT intpow(MYFLT x, int32_t n) /* Binary power function */
{
  if (n < 0) {
    n = -n;
    x = FL(1.0) / x;
  }
  return intpow1(x, n);
}