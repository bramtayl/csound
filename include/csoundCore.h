/*
    csoundCore.h:

    Copyright (C) 1991-2006 Barry Vercoe, John ffitch, Istvan Varga

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

#ifndef CSOUNDCORE_H
#define CSOUNDCORE_H

#if defined(__EMSCRIPTEN__) && !defined(EMSCRIPTEN)
#define EMSCRIPTEN
#endif

#include "sysdep.h"
#if !defined(EMSCRIPTEN) && !defined(CABBAGE)
#if defined(HAVE_PTHREAD)
#include <pthread.h>
#endif
#endif
#include "cs_par_structs.h"
#include <stdarg.h>
#include <setjmp.h>
#include "csound_type_system.h"
#include "csound.h"
#include "cscore.h"
#include "csound_data_structures.h"
#include "csound_standard_types.h"
#include "pools.h"
#include "soundfile.h"
#include "csoundCore_common.h"

#ifdef __cplusplus
extern "C" {
#endif /*  __cplusplus */

  /**
   * Contains all function pointers, data, and data pointers required
   * to run one instance of Csound.
   *
   * \b PUBLIC functions in CSOUND_
   * These are used by plugins to access the
   * Csound library functionality without the requirement
   * of compile-time linkage to the csound library
   * New functions only need to be added here if
   * they are required by plugins.
   */
  struct CSOUND_;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* CSOUNDCORE_H */
