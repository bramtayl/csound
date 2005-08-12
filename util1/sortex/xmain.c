/*
    xmain.c

    Copyright (C) 1991 Barry Vercoe, John ffitch

    This file is part of Csound.

    Csound is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Csound is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Csound; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "csound.h"                                /*   XMAIN.C  */

PUBLIC int scxtract(CSOUND *, FILE *, FILE *, FILE *);

int main(int ac, char **av)         /* stdio stub for standalone extract */
                                    /*     first opens the control xfile */
{
    CSOUND *csound;
    FILE    *xfp;

    csound = (CSOUND*) csoundCreate(NULL);
    csoundPreCompile(csound);
    ac--;  av++;
    if (ac != 1) {
      fprintf(stderr, "usage: extract xfile <in >out\n");
      exit(1);
    }
    if ((xfp = fopen(*av, "r")) == NULL) {
      fprintf(stderr, "extract: can't open %s\n", *av);
      exit(1);
    }
    return (scxtract(csound, stdin, stdout, xfp));
}

