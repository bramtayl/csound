#include "memalloc_internal.h"
#include "memalloc.h"
#include "csoundCore_internal.h"

void *mmallocDebug(CSOUND *csound, size_t size, char *file, int line)
{
    void *ans = mmalloc(csound,size);
    printf("Alloc %p (%zu) %s:%d\n", ans, size, file, line);
    return ans;
}

void *mcallocDebug(CSOUND *csound, size_t size, char *file, int line)
{
    void *ans = mcalloc(csound,size);
    printf("Alloc %p (%zu) %s:%d\n", ans, size, file, line);
    return ans;
}

void *mreallocDebug(CSOUND *csound, void *oldp, size_t size, char *file, int line)
{
    void *p = mrealloc(csound, oldp, size);
    printf("Realloc %p->%p (%zu) %s:%d\n", oldp, p, size, file, line);
    return p;
}

void mfreeDebug(CSOUND *csound, void *ans, char *file, int line)
{
    printf("Free %p %s:%d\n", ans, file, line);
    mfree(csound,ans);
}

void memRESET(CSOUND *csound)
{
    memAllocBlock_t *pp, *nxtp;

    pp = (memAllocBlock_t*) MEMALLOC_DB;
    MEMALLOC_DB = NULL;
    while (pp != NULL) {
      nxtp = pp->nxt;
#ifdef MEMDEBUG
      pp->magic = 0;
#endif
      CS_FREE((void*) pp);
      pp = nxtp;
    }
}
