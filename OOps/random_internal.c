#include "random_internal.h"
#include "csoundCore_internal.h"
#include "random.h"

/* called from csoundPreCompile() */

void csound_init_rand(CSOUND *csound)
{
    uint32_t  tmp;

    csound->csRandState = &(csound->randState_);
    csound->randSeed1 = 15937;
    tmp = (uint32_t) csoundGetRandomSeedFromTime();
    while (tmp >= (uint32_t) 0x7FFFFFFE)
      tmp -= (uint32_t) 0x7FFFFFFE;
    csound->randSeed2 = ((int) tmp + 1);
    csoundSeedRandMT(&(csound->randState_), NULL, (uint32_t) 5489);
}
