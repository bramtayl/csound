#include "libsnd_u_internal.h"
#include "csoundCore_internal.h"

void dbfs_init(CSOUND *csound, MYFLT dbfs)
{
    csound->dbfs_to_float = FL(1.0) / dbfs;
    csound->e0dbfs = dbfs;
    /* probably want this message written just before note messages start... */
    /* VL: printing too early does not allow us to switch this off
       better print this when the engine is ready to run.
     */
    // csoundMessage(csound, Str("0dBFS level = %.1f\n"), dbfs);

}