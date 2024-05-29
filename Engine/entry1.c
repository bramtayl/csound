/*
  entry1.c:

  Copyright (C) 1991 Barry Vercoe, John ffitch

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

#include "entry1.h"             /*                      ENTRY1.C        */
#include "interlocks.h"
#include "csoundCore_internal.h"         /*                      ENTRY1.H        */
#include "oload.h"

/* thread vals, where isub=1, ksub=2:
   0 =     1  OR   2  (B out only)
   1 =     1
   2 =             2
   3 =     1  AND  2
*/

/* inarg types include the following:
   i       irate scalar
   k       krate scalar
   a       arate vector
   f       frequency variable
   w       spectral variable
   x       krate scalar or arate vector
   S       String
   T       String or i-rate
   U       String or i/k-rate
   B       Boolean k-rate
   b       Boolean i-rate; internally generated as required
   l       Label
   .       required arg of any-type
   and codes
   ?       optional arg of any-type
   m       begins an indef list of iargs (any count)
   M       begins an indef list of args (any count/rate i,k,a)
   N       begins an indef list of args (any count/rate i,k,a,S)
   o       optional i-rate, defaulting to  0
   p              "             "          1
   q              "             "         10
   v              "             "          .5
   j              "             "         -1
   h              "             "        127
   O       optional k-rate, defaulting to  0
   J              "             "         -1
   V              "             "          .5
   P              "             "          1
   W       begins indef list of Strings (any count)
   y       begins indef list of aargs (any count)
   z       begins indef list of kargs (any count)
   Z       begins alternating kakaka...list (any count)
*/

/* outarg types include:
 i, k, a, S as  above
 *       multiple out args of any-type
 m       multiple out aargs
 z       multiple out kargs
 I       multiple out irate (not implemented yet)
 s       deprecated (use a or k as required)
 X       multiple args (a, k, or i-rate)     IV - Sep 1 2002
 N       multiple args (a, k, i, or S-rate)
 F       multiple args (f-rate)#
*/

/* inargs and outargs may also be arrays, e.g. "a[b]" is an array of
   arate vectors. Then for polymorphic opcode entries, "opcode.a" is
   for arate vectors, and "opcode.A" is for arrays of arate vectors.
*/

OENTRY opcodlst_1[] = {
  /* opcode   dspace  flags  thread  outarg  inargs  isub    ksub    asub    */
  { "",     0,      0,    0,      "",     "",     NULL, NULL, NULL, NULL },
  { "instr",  0,    0,      0,      "",     "",   NULL, NULL, NULL, NULL },
  { "endin",  0,    0,      0,      "",     "",   NULL, NULL, NULL, NULL },
  /* IV - Sep 8 2002 */
  { "opcode",  0,    0,      0,      "",     "",   NULL, NULL, NULL, NULL },
  { "endop",   0,    0,      0,      "",     "",   NULL, NULL, NULL, NULL },
  { "declare", 0,    0,      0,      "",     "",   NULL, NULL, NULL, NULL },
  { "pset",   S(PVSET),   0,0,      "",     "m",  NULL, NULL, NULL, NULL },
  /*  { "nlalp", S(NLALP),0,     3,     "a",  "akkoo", nlalp_set, nlalp }, */
  //  { "##globallock",   S(GLOBAL_LOCK_UNLOCK),0, 3, "", "i",
  //    globallock,   globallock,   NULL},
  //  { "##globalunlock", S(GLOBAL_LOCK_UNLOCK),0, 3, "", "i",
  //    globalunlock, globalunlock, NULL},
  /* terminate list */
  {  NULL, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL}
};
