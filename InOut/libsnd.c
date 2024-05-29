/*
    sndlib.c:

    Copyright (C) 2004 John ffitch

    This file is part of Csound.

    The Csound Library is free software; you can redistribute it
    and/or modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    Csound is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with Csound; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
    02110-1301 USA
*/

#include "csoundCore_internal.h"                 /*             SNDLIB.C         */
#include "soundio.h"
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>
#include "libsnd_u.h"
#include "memalloc.h"
#include "envvar_public.h"
#include "libsnd.h"
#include "text.h"
#include "libsnd_internal.h"
#include "libsnd.h"
#include "text.h"
#include "libsnd_internal.h"

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

PUBLIC MYFLT *csoundGetInputBuffer(CSOUND *csound)
{
    return STA_LIBSND(inbuf);
}

PUBLIC MYFLT *csoundGetOutputBuffer(CSOUND *csound)
{
    return STA_LIBSND(outbuf);
}
