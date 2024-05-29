#include "linevent_public.h"
#include "linevent.h"
#include "memalloc.h"
#include "musmon.h"
#include "text.h"

static CS_NOINLINE int linevent_alloc(CSOUND *csound, int reallocsize)
{
    volatile jmp_buf tmpExitJmp;
    int         err;
    unsigned int tmp;

    if (reallocsize > 0) {
      /* VL 20-11-17 need to record the STA_LINEVENT(Linep) offset
         in relation to STA_LINEVENT(Linebuf) */
      tmp = (STA_LINEVENT(Linep) - STA_LINEVENT(Linebuf));
      STA_LINEVENT(Linebuf) = (char *) mrealloc(csound,
                                              (void *) STA_LINEVENT(Linebuf), reallocsize);

      STA_LINEVENT(linebufsiz) = reallocsize;
      STA_LINEVENT(Linebufend) = STA_LINEVENT(Linebuf) + STA_LINEVENT(linebufsiz);
      /* VL 20-11-17 so we can place it in the correct position
         after reallocation */
      STA_LINEVENT(Linep) =  STA_LINEVENT(Linebuf) + tmp;
    } else if (STA_LINEVENT(Linebuf)==NULL) {
       STA_LINEVENT(linebufsiz) = LBUFSIZ1;
       STA_LINEVENT(Linebuf) = (char *) mcalloc(csound, STA_LINEVENT(linebufsiz));
    }
    if (STA_LINEVENT(Linebuf) == NULL) {
       return 1;
    }
    //csoundMessage(csound, "1. realloc: %d\n", reallocsize);
    if (STA_LINEVENT(Linep)) return 0;
    csound->Linefd = -1;
    memcpy((void*) &tmpExitJmp, (void*) &csound->exitjmp, sizeof(jmp_buf));
    if ((err = setjmp(csound->exitjmp)) != 0) {
      memcpy((void*) &csound->exitjmp, (void*) &tmpExitJmp, sizeof(jmp_buf));
      //csound->lineventGlobals = NULL;
      return -1;
    }


    memcpy((void*) &csound->exitjmp, (void*) &tmpExitJmp, sizeof(jmp_buf));
    STA_LINEVENT(prve).opcod = ' ';
    STA_LINEVENT(Linebufend) = STA_LINEVENT(Linebuf) + STA_LINEVENT(linebufsiz);
    STA_LINEVENT(Linep) = STA_LINEVENT(Linebuf);
    csoundRegisterSenseEventCallback(csound, sensLine, NULL);

    return 0;
}

/* insert text from an external source,
   to be interpreted as if coming in from stdin/Linefd for -L */

void csoundInputMessageInternal(CSOUND *csound, const char *message)
{
    int32  size = (int32) strlen(message);
#if 1
    int n;
#endif

    if ((n=linevent_alloc(csound, 0)) != 0) return;

    if (!size) return;
    if (UNLIKELY((STA_LINEVENT(Linep) + size) >= STA_LINEVENT(Linebufend))) {
      int extralloc = STA_LINEVENT(Linep) + size - STA_LINEVENT(Linebufend);
      csoundMessage(csound, "realloc %d\n", extralloc);
      // csoundMessage(csound, "extralloc: %d %d %d\n",
      //                 extralloc, size, (int)(STA_LINEVENT(Linebufend) - STA_LINEVENT(Linep)));
      // FIXME -- Coverity points out that this test is always false
      // and n is never used
#if 1
      if ((n=linevent_alloc(csound, (STA_LINEVENT(linebufsiz) + extralloc))) != 0) {
        csoundErrorMsg(csound, Str("LineBuffer Overflow - "
                                   "Input Data has been Lost"));
        return;
      }
#else
      (void) linevent_alloc(csound, (STA_LINEVENT(linebufsiz) + extralloc));

#endif
    }
    //csoundMessage(csound, "%u = %u\n", (STA_LINEVENT(Linep) + size),  STA_LINEVENT(Linebufend) );
    memcpy(STA_LINEVENT(Linep), message, size);
    if (STA_LINEVENT(Linep)[size - 1] != (char) '\n')
      STA_LINEVENT(Linep)[size++] = (char) '\n';
    STA_LINEVENT(Linep) += size;
}
