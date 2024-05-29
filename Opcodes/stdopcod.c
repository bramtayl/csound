/*
    stdopcod.c:

    Copyright (c) 2005 Istvan Varga

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

#include "stdopcod.h"
#include "memalloc.h"
#include "text.h"
#include "bbcut.h"
#include "biquad.h"
#include "butter.h"
#include "clfilt.h"
#include "dam.h"
#include "flanger.h"
#include "follow.h"
#include "fout.h"
#include "ftgen.h"
#include "gab/gab.h"
#include "gab/vectorial.h"
#include "grain.h"
#include "locsig.h"
#include "lowpassr.h"
#include "midiops2.h"
#include "midiops3.h"
#include "newfils.h"
#include "nlfilt.h"
#include "oscbnk.h"
#include "pluck.h"
#include "repluck.h"
#include "reverbsc.h"
#include "sndloop.h"
#include "sndwarp.h"
#include "space.h"
#include "spat3d.h"
#include "syncgrain.h"
#include "ugens7.h"
#include "ugens9.h"
#include "ugensa.h"
#include "uggab.h"
#include "ugmoss.h"
#include "ugnorman.h"
#include "ugsc.h"
#include "wave-terrain.h"

/* PUBLIC int32_t csoundModuleCreate(CSOUND *csound)
{
    (void) csound;
    return 0;
}
*/

int32_t stdopc_ModuleInit(CSOUND *csound)
{
    STDOPCOD_GLOBALS  *p;
    int32_t               err = 0;

    if (UNLIKELY(csound->stdOp_Env != NULL)) {
      csoundErrorMsg(csound, Str("stdopcod.c: error: globals already allocated"));
      return CSOUND_ERROR;
    }
    csound->stdOp_Env = mcalloc(csound, sizeof(STDOPCOD_GLOBALS));

    p = (STDOPCOD_GLOBALS*) csound->stdOp_Env;
    p->csound = csound;
    /* fout.c */
    p->file_opened = (struct fileinTag*) NULL;
    p->file_num = -1;
    /*p->buf = (MYFLT*) NULL;*/
    /* ugnorman.c */
    p->atsbufreadaddr = NULL;
    err |= bbcut_init_(csound);
    err |= biquad_init_(csound);
    err |= butter_init_(csound);
    err |= clfilt_init_(csound);
    err |= dam_init_(csound);
    err |= flanger_init_(csound);
    err |= follow_init_(csound);
    err |= fout_init_(csound);
    err |= ftgen_init_(csound);
    err |= gab_gab_init_(csound);
    err |= gab_vectorial_init_(csound);
    err |= grain_init_(csound);
    err |= locsig_init_(csound);
    err |= lowpassr_init_(csound);
    err |= midiops2_init_(csound);
    err |= midiops3_init_(csound);
    err |= newfils_init_(csound);
    err |= nlfilt_init_(csound);
    err |= oscbnk_init_(csound);
    err |= pluck_init_(csound);
    err |= repluck_init_(csound);
    err |= reverbsc_init_(csound);
    err |= sndloop_init_(csound);
    err |= sndwarp_init_(csound);
    err |= space_init_(csound);
    err |= spat3d_init_(csound);
    err |= syncgrain_init_(csound);
    err |= ugens7_init_(csound);
    err |= ugens9_init_(csound);
    err |= ugensa_init_(csound);
    err |= uggab_init_(csound);
    err |= ugmoss_init_(csound);
    err |= ugnorman_init_(csound);
    err |= ugsc_init_(csound);
    err |= wave_terrain_init_(csound);
    return (err ? CSOUND_ERROR : CSOUND_SUCCESS);
}

/*
PUBLIC int32_t csoundModuleInfo(void)
{
    return ((CS_APIVERSION << 16) + (CS_APISUBVER << 8) + (int32_t) sizeof(MYFLT));
}
*/

