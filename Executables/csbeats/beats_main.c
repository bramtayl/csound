/*
    beats/main.c:

    Copyright (C) 2009,2010 John ffitch

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "beats.tab.h"
#include "beats_main.h"
#include "beatslex.yy.h"

FILE *myout;
int debug = 0;

double pt[13] = { 8.1757989156,  8.6619572180,  9.1770239974,  9.7227182413,
                 10.3008611535, 10.9133822323, 11.5623257097, 12.2498573744,
                 12.9782717994, 13.7500000000, 14.5676175474, 15.4338631643,
                 16.3516084259
};

INSTR *instr;

int main(int argc, char **argv)
{
    char buf[80];
    time_t timep;
    struct tm tm;
    instr = NULL;
    bpm = 60;
    permeasure = 4;
    yydebug = 0;
    yyin = stdin;
    /* Argument decode:  A litte dodgy.  In csound should have zero args */
    if (argc==3) {
      yyin = fopen(argv[1], "r");
      myout = fopen(argv[2], "w");
    }
    else if (argc==4) {
      debug = 1;
      yyin = fopen(argv[2], "r");
      myout = fopen(argv[3], "w");
    }
    else if (argc==2) {
      yydebug = debug = 1;
      myout = stdout;
    } else
      myout = stdout;
    if (yyin==NULL || myout==NULL) {
      fprintf(stderr, "Failed to open files\n");
      exit(1);
    }
    time(&timep);

#ifndef __gnu_linux__
    {
      struct tm *date_ptr = localtime(&timep);
      memcpy(&tm, date_ptr, sizeof(struct tm));
    }
#else
    localtime_r(&timep, &tm);
#endif

    strftime(buf, 80, "%a %Y %b %d %H:%M:%S", &tm);
    fprintf(myout, ";;; Generated by beats on %s\n\n", buf);
    instr = (INSTR*) calloc(1, sizeof(INSTR));
    instr->next = instr;
    instr->p = (double*) calloc(6, sizeof(double));
    instr->largest = 5;
    yyparse();
    return 0;
}

INSTR *find_instr(int n)
{                               /* search circular list fot instr n */
    INSTR *p = instr;
    do {
      if (n == p->n) return (instr = p);
      p = p->next;
    } while (p != instr);
    /* Not there so add at start of list */
    p = (INSTR*) calloc(1, sizeof(INSTR));
    p->next = instr->next;
    instr->next = p;
    p->n = n;
    return (instr=p);
}

void print_instr_structure(void)
{
    INSTR *p = instr;
    fprintf(stderr, "Instruments\n");
    do {
      fprintf(stderr, "%d: last p is %d, array is at %p\n", p->n, p->largest, p->p);
      if (p->largest>5) {
        int j;
        for (j=6; j<=p->largest; j++)
          fprintf(stderr, " %d->%lf", j, p->p[j]);
        fprintf(stderr, "\n");
        p = p->next;
      }
    } while (p!=instr);
    fprintf(stderr, "\n end \n");
}
