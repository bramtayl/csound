#include "parse_param.h"

uint64_t make_location(PRE_PARM *qq)
{
    int d = qq->depth;
    uint64_t loc = 0;
    int n = (d>8?d-7:0);
    for (; n<=d; n++) {
      loc = (loc<<8)+(qq->lstack[n]);
    }
    return loc;
}