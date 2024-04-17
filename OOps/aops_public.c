#include "aops_public.h"

#include "aops.h"
#include "csound.h"

MYFLT csoundPow2(CSOUND *csound, MYFLT a) {
  (void)(csound);
  /* int32_t n; */
  if (a > POW2MAX)
    a = POW2MAX;
  else if (a < -POW2MAX)
    a = -POW2MAX;
  return POWER(FL(2.0), a);
  /* 4096 * 15 */
  /* n = (int32_t)MYFLT2LRND(a * FL(POW2TABSIZI)) + POW2MAX*POW2TABSIZI; */
  /* return ((MYFLT) (1UL << (n >> 12)) * csound->powerof2[n &
   * (POW2TABSIZI-1)]); */
}
