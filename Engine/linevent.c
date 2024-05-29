/*
    linevent_public.c:

    Copyright (C) 1991 Barry Vercoe, John ffitch, matt ingalls

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

#include "csoundCore_internal.h"     /*                              LINEVENT.C      */
#include <ctype.h>

#ifdef MSVC
#include <fcntl.h>
#endif

#include "linevent.h"
#include "linevent_public.h"
#include "rdscor.h"
#include "namedins_public.h"
#include "memalloc.h"
#include "fgens_public.h"
#include "insert_public.h"
#include "musmon.h"
#include "text.h"
#include "threadsafe_public.h"

#ifdef PIPES
# if defined(SGI) || defined(__gnu_linux__) || defined(NeXT) || defined(__MACH__)
#  define _popen popen
#  define _pclose pclose
# elif defined(__BEOS__) ||  defined(__HAIKU__) || defined(__MACH__)
#  include <stdio.h>
#  define _popen popen
#  define _pclose pclose
# else
   FILE *_popen(const char *, const char *);
# endif
#endif

#define LF        '\n'

/* typedef struct { */
/*     char    *Linep, *Linebufend; */
/*     FILE    *Linecons; */
/*     int     stdmode; */
/*     EVTBLK  prve; */
/*     char    Linebuf[LBUFSIZ]; */
/* } LINEVENT_GLOBALS; */

#define MAXSTR 1048576 /* 1MB */

#ifndef O_NDELAY
#define O_NDELAY 0
#endif

void RTLineset(CSOUND *csound)      /* set up Linebuf & ready the input files */
{                                   /*     callable once from musmon.c        */
    OPARMS  *O = csound->oparms;
    /* csound->lineventGlobals = (LINEVENT_GLOBALS*) */
    /*                            mcalloc(csound, */
    /*                            sizeof(LINEVENT_GLOBALS)); */

    STA_LINEVENT(linebufsiz) = LBUFSIZ1;
    STA_LINEVENT(Linebuf) = (char *) mcalloc(csound, STA_LINEVENT(linebufsiz));
    STA_LINEVENT(orchestrab) = (char *) mcalloc(csound, MAXSTR);
    STA_LINEVENT(orchestra) = STA_LINEVENT(orchestrab);
    STA_LINEVENT(prve).opcod = ' ';
    STA_LINEVENT(Linebufend) = STA_LINEVENT(Linebuf) + STA_LINEVENT(linebufsiz);
    STA_LINEVENT(Linep) = STA_LINEVENT(Linebuf);
    if (strcmp(O->Linename, "stdin") == 0) {
#if defined(DOSGCC) || defined(_WIN32)
      setvbuf(stdin, NULL, _IONBF, 0);
      /* WARNING("-L stdin:  system has no fcntl function to get stdin"); */
#else
      STA_LINEVENT(stdmode) = fcntl(csound->Linefd, F_GETFL, 0);
      if (UNLIKELY(fcntl(csound->Linefd, F_SETFL, STA_LINEVENT(stdmode) | O_NDELAY) < 0))
        csoundDie(csound, Str("-L stdin fcntl failed"));
#endif
    }
#ifdef PIPES
    else if (UNLIKELY(O->Linename[0] == '|')) {
      csound->Linepipe = _popen(&(O->Linename[1]), "r");
      if (LIKELY(csound->Linepipe != NULL)) {
        csound->Linefd = fileno(csound->Linepipe);
        setvbuf(csound->Linepipe, NULL, _IONBF, 0);
      }
      else csoundDie(csound, Str("Cannot open %s"), O->Linename);
    }
#endif
#define MODE ,0
    else
      if (UNLIKELY((csound->Linefd=open(O->Linename, O_RDONLY|O_NDELAY MODE)) < 0))
        csoundDie(csound, Str("Cannot open %s"), O->Linename);
    if(csound->oparms->odebug)
    csoundMessage(csound, Str("stdmode = %.8x Linefd = %d\n"),
                    STA_LINEVENT(stdmode), csound->Linefd);
    csoundRegisterSenseEventCallback(csound, sensLine, NULL);
}

#ifdef PIPES
int _pclose(FILE*);
#endif

void RTclose(CSOUND *csound)
{
    if (csound->oparms->Linein == 0)
      return;
    csound->oparms->Linein = 0;
    if(csound->oparms->odebug)
    csoundMessage(csound, Str("stdmode = %.8x Linefd = %d\n"),
                    STA_LINEVENT(stdmode), csound->Linefd);
#ifdef PIPES
    if (csound->oparms->Linename[0] == '|')
      _pclose(csound->Linepipe);
    else
#endif
      {
        if (strcmp(csound->oparms->Linename, "stdin") != 0)
          close(csound->Linefd);
#if !defined(DOSGCC) && !defined(_WIN32)
        else
          if (UNLIKELY(fcntl(csound->Linefd, F_SETFL, STA_LINEVENT(stdmode))))
            csoundDie(csound, Str("Failed to set file status\n"));
#endif
      }

//mfree(csound, csound->lineventGlobals);
//csound->lineventGlobals = NULL;
}

/* does string segment contain LF? */

static inline int containsLF(char *cp, char *endp)
{
    while (cp < endp) {
      if (UNLIKELY(*cp++ == LF))
        return 1;
    }
    return 0;
}

/* accumlate RT Linein buffer, & place completed events in EVTBLK */
/* does more syntax checking than rdscor, since not preprocessed  */

void sensLine(CSOUND *csound, void *userData)
{
    char    *cp, *Linestart, *Linend;
    int     c, cm1, cpp1, n, pcnt, oflag = STA_LINEVENT(oflag);
    IGN(userData);

    while (1) {
      if(STA_LINEVENT(oflag) > oflag) break;
      Linend = STA_LINEVENT(Linep);
      if (csound->Linefd >= 0) {
        n = read(csound->Linefd, Linend, STA_LINEVENT(Linebufend) - Linend);
        Linend += (n > 0 ? n : 0);
      }
      if (Linend <= STA_LINEVENT(Linebuf))
        break;
      Linestart = STA_LINEVENT(Linebuf);
      cp = Linestart;

      while (containsLF(Linestart, Linend)) {
        EVTBLK  e;
        char    *sstrp = NULL;
        int     scnt = 0;
        int     strsiz = 0;
        memset(&e, 0, sizeof(EVTBLK));
        e.strarg = NULL; e.scnt = 0;
        c = *cp;
        while (isblank(c))              /* skip initial white space */
          c = *(++cp);
        if (c == LF) {                  /* if null line, bugout     */
          Linestart = (++cp);
          continue;
        }
        cm1 = *(cp-1);
        cpp1 = *(cp+1);

        /* new orchestra input
         */
        if(STA_LINEVENT(oflag)) {
          if(c == '}' && cm1 != '}' && cpp1 != '}') {
            STA_LINEVENT(oflag) = 0;
            STA_LINEVENT(orchestra) = STA_LINEVENT(orchestrab);
            csoundCompileOrc(csound, STA_LINEVENT(orchestrab));
            csoundMessage(csound, "::compiling orchestra::\n");
            Linestart = (++cp);
            continue;
          }
          else {
            char *pc;
            memcpy(STA_LINEVENT(orchestra), Linestart, Linend - Linestart);
            STA_LINEVENT(orchestra) += (Linend - Linestart);
            *STA_LINEVENT(orchestra) = '\0';
            STA_LINEVENT(oflag)++;
            if((pc = strrchr(STA_LINEVENT(orchestrab), '}')) != NULL) {

              if(*(pc-1) != '}') {
              *pc = '\0';
               cp = strrchr(Linestart, '}');
              } else {
               Linestart = Linend;
              }
              } else {
              Linestart = Linend;
            }
            continue;
          }
        } else if(c == '{') {
          STA_LINEVENT(oflag) = 1;
          csoundMessage(csound,
                          "::reading orchestra, use '}' to terminate::\n");
          cp++;
          continue;
        }

        switch (c) {                    /* look for legal opcode    */
        case 'e':                       /* Quit realtime            */
        case 'i':
        case 'q':
        case 'f':
        case 'a':
        case 'd':
          e.opcod = c;
          break;
        default:
          csoundErrorMsg(csound, Str("unknown opcode %c"), c);
          goto Lerr;
        }                                       /* for params that follow:  */
        pcnt = 0;
        do {
          char  *newcp;
          do {                                  /* skip white space */
            c = *(++cp);
          } while (isblank(c));
          if (c == LF)
            break;
          pcnt++;
          if (c == '"') {                       /* if find character string */
            if (e.strarg == NULL)
              e.strarg = mmalloc(csound, strsiz=SSTRSIZ);
            sstrp = e.strarg;
            n = scnt;
            while (n-->0) sstrp += strlen(sstrp)+1;
            n = 0;

            while ((c = *(++cp)) != '"') {
              /* VL: allow strings to be multi-line */
              // if (UNLIKELY(c == LF)) {
              //  csoundErrorMsg(csound, Str("unmatched quotes"));
              //  goto Lerr;
              //}

              if(c == '\\') {
                cp++;
                if(*cp == '"') c = *cp; /* if it is a double quote */
                /* otherwise we ignore it */
                else cp--;
              }
              sstrp[n++] = c;                   /*   save in private strbuf */

              if (UNLIKELY((sstrp-e.strarg)+n >= strsiz-10)) {
                e.strarg = mrealloc(csound, e.strarg, strsiz+=SSTRSIZ);
                sstrp = e.strarg+n;
              }
            }
            sstrp[n] = '\0';
            {
              union {
                MYFLT d;
                int32 i;
              } ch;
              ch.d = SSTRCOD; ch.i += scnt++;
              e.p[pcnt] = ch.d;           /* set as string with count */
            }
            e.scnt = scnt;
            //printf("string: %s\n", sstrp);
            continue;
          }
          if (UNLIKELY(!(isdigit(c) || c == '+' || c == '-' || c == '.')))
            goto Lerr;
          if (c == '.' &&                       /*  if lone dot,       */
              (isblank(n = cp[1]) || n == LF)) {
            if (UNLIKELY(e.opcod != 'i' ||
                         STA_LINEVENT(prve).opcod != 'i' || pcnt > STA_LINEVENT(prve).pcnt)) {
              csoundErrorMsg(csound, Str("dot carry has no reference"));
              goto Lerr;
            }                                   /*        pfld carry   */
            e.p[pcnt] = STA_LINEVENT(prve).p[pcnt];
            if (UNLIKELY(isstrcod(e.p[pcnt]))) {
              csoundErrorMsg(csound, Str("cannot carry string p-field"));
              goto Lerr;
            }
            continue;
          }
          e.p[pcnt] = (MYFLT) cs_strtod(cp, &newcp);
          cp = newcp - 1;
        } while (pcnt < PMAX);
        if (e.opcod =='f' && e.p[1]<FL(0.0)); /* an OK case */
        else  /* Check for sufficient pfields (0-based, opcode counted already). */
          if (UNLIKELY(pcnt < 2 && e.opcod != 'e')) {
            csoundErrorMsg(csound, Str("too few pfields (%d)"), pcnt + 1);
            goto Lerr;
          }
        if (UNLIKELY(pcnt > 1 && e.p[2] < FL(0.0))) {
          csoundErrorMsg(csound, Str("-L with negative p2 illegal"));
          goto Lerr;
        }
        e.pcnt = pcnt;                          /*   &  record pfld count    */
        if (e.opcod == 'i') {                   /* do carries for instr data */
          memcpy((void*) &STA_LINEVENT(prve), (void*) &e,
                 (size_t) ((char*) &(e.p[pcnt + 1]) - (char*) &e));
          /* FIXME: how to carry string args ? */
          STA_LINEVENT(prve).strarg = NULL;
        }
        if (UNLIKELY(pcnt >= PMAX && c != LF)) {
          csoundErrorMsg(csound, Str("too many pfields"));
          while (*(++cp) != LF)                 /* flush any excess data     */
            ;
        }
        Linestart = (++cp);
        insert_score_event_at_sample(csound, &e, csound->icurTime);
        continue;
      Lerr:
        n = cp - Linestart;                     /* error position */
        while (*cp != LF)
          cp++;                                 /* go on to LF    */
        *cp = '\0';                             /*  & insert NULL */
        csoundErrorMsg(csound, Str("illegal RT scoreline:\n%s\n%*s"),
                                 Linestart, n + 1, "^");  /* mark the error */
        Linestart = (++cp);
      }
      if (Linestart != &(STA_LINEVENT(Linebuf)[0])) {
        int len = (int) (Linend - Linestart);
        /* move any remaining characters to the beginning of the buffer */
        for (n = 0; n < len; n++)
          STA_LINEVENT(Linebuf)[n] = Linestart[n];
        n = (int) (Linestart - &(STA_LINEVENT(Linebuf)[0]));
        STA_LINEVENT(Linep) -= n;
        Linend -= n;
      }
      if (Linend == STA_LINEVENT(Linep))      /* return if no more data is available  */
        break;
      STA_LINEVENT(Linep) = Linend;                       /* accum the chars          */
    }

}

/* send a lineevent from the orchestra -matt 2001/12/07 */

static const char *errmsg_1 =
  Str_noop("event: param 1 must be \"a\", \"i\", \"q\", \"f\", \"d\", or \"e\"");
static const char *errmsg_2 =
  Str_noop("event: string name is allowed only for \"i\", \"d\", and \"q\" events");

int eventOpcode_(CSOUND *csound, LINEVENT *p, int insname, char p1)
{
    EVTBLK  evt;
    int     i;
    char    opcod;
    memset(&evt, 0, sizeof(EVTBLK));

    if (p1==0)
         opcod = *((STRINGDAT*) p->args[0])->data;
    else  opcod = p1;

    if (UNLIKELY((opcod != 'a' && opcod != 'i' && opcod != 'q' && opcod != 'f' &&
                  opcod != 'e' && opcod != 'd')
                 /*|| ((STRINGDAT*) p->args[0])->data[1] != '\0'*/))
      return csoundPerfError(csound, &(p->h), "%s", Str(errmsg_1));
    evt.strarg = NULL; evt.scnt = 0;
    evt.opcod = opcod;
    if (p->flag==1) evt.pcnt = p->argno-2;
    else
      evt.pcnt = p->INOCOUNT - 1;

    /* IV - Oct 31 2002: allow string argument */
    if (evt.pcnt > 0) {
      if (insname) {
        int res;
        if (UNLIKELY(evt.opcod != 'i' && evt.opcod != 'q' && opcod != 'd'))
          return csoundPerfError(csound, &(p->h), "%s", Str(errmsg_2));
        res = strarg2insno(csound, ((STRINGDAT*) p->args[1])->data, 1);
        if (UNLIKELY(res == NOT_AN_INSTRUMENT)) return NOTOK;
        evt.p[1] = (MYFLT) res;
        evt.strarg = NULL; evt.scnt = 0;
      }
      else {
        int res;
        if (isstrcod(*p->args[1])) {
          res = strarg2insno(csound,
                                     get_arg_string(csound, *p->args[1]), 1);
          if (UNLIKELY(res == NOT_AN_INSTRUMENT)) return NOTOK;
          evt.p[1] = (MYFLT)res;
        } else {                  /* Should check for valid instr num here */
          MYFLT insno = FABS(*p->args[1]);
          evt.p[1] = *p->args[1];
          if (UNLIKELY((opcod == 'i' || opcod == 'd') && (insno ==0 ||
                       insno > csound->engineState.maxinsno ||
                       !csound->engineState.instrtxtp[(int)insno]))) {
            csoundMessage(csound, Str("WARNING: Cannot Find Instrument %d. No action."),
                           (int) insno);
            return OK;
          }
        }
        evt.strarg = NULL; evt.scnt = 0;
      }
      for (i = 2; i <= evt.pcnt; i++)
        evt.p[i] = *p->args[i];
    }

    if(opcod == 'd') {
      evt.opcod = 'i';
      evt.p[1] *= -1;
    }

    if (UNLIKELY(insert_score_event_at_sample(csound, &evt, csound->icurTime) != 0))
      return csoundPerfError(csound, &(p->h),
                               Str("event: error creating '%c' event"),
                               opcod);
    return OK;
}

int eventOpcode(CSOUND *csound, LINEVENT *p)
{
    return eventOpcode_(csound, p, 0, 0);
}

int eventOpcode_S(CSOUND *csound, LINEVENT *p)
{
    return eventOpcode_(csound, p, 1, 0);
}



/* i-time version of event opcode */

int eventOpcodeI_(CSOUND *csound, LINEVENT *p, int insname, char p1)
{
    EVTBLK  evt;
    int     i, err = 0;
    char    opcod;
    memset(&evt, 0, sizeof(EVTBLK));

    if (p1==0)
         opcod = *((STRINGDAT*) p->args[0])->data;
    else opcod = p1;
    if (UNLIKELY((opcod != 'a' && opcod != 'i' && opcod != 'q' && opcod != 'f' &&
                  opcod != 'e' && opcod != 'd')
                 /*|| ((STRINGDAT*) p->args[0])->data[1] != '\0'*/))
      return csoundInitError(csound, "%s", Str(errmsg_1));
    evt.strarg = NULL; evt.scnt = 0;
    evt.opcod = opcod;
    if (p->flag==1) evt.pcnt = p->argno-1;
    else
      evt.pcnt = p->INOCOUNT - 1;
    /* IV - Oct 31 2002: allow string argument */
    if (evt.pcnt > 0) {
      if (insname) {
        int res;
        if (UNLIKELY(evt.opcod != 'i' && evt.opcod != 'q' && opcod != 'd'))
          return csoundInitError(csound, "%s", Str(errmsg_2));
        res = strarg2insno(csound,((STRINGDAT *)p->args[1])->data, 1);
        if (UNLIKELY(res == NOT_AN_INSTRUMENT)) return NOTOK;
        evt.p[1] = (MYFLT)res;
        evt.strarg = NULL; evt.scnt = 0;
        for (i = 2; i <= evt.pcnt; i++)
           evt.p[i] = *p->args[i];
      }
      else {
        evt.strarg = NULL; evt.scnt = 0;
        if (isstrcod(*p->args[1])) {
          int res = strarg2insno(csound,
                                         get_arg_string(csound, *p->args[1]), 1);
          if (UNLIKELY(evt.p[1] == NOT_AN_INSTRUMENT)) return NOTOK;
          evt.p[1] = (MYFLT)res;
        }
        else {                  /* Should check for valid instr num here */
          MYFLT insno = FABS(*p->args[1]);
          evt.p[1] = *p->args[1];
          if (UNLIKELY((opcod == 'i' || opcod == 'd') && (insno ==0 ||
                       insno > csound->engineState.maxinsno ||
                       !csound->engineState.instrtxtp[(int)insno]))) {
            csoundMessage(csound, Str("WARNING: Cannot Find Instrument %d. No action."),
                           (int) insno);
            return OK;
          }
          evt.strarg = NULL; evt.scnt = 0;
        }
        for (i = 2; i <= evt.pcnt; i++)
          evt.p[i] = *p->args[i];
      }

    }
    if(opcod == 'd') {
      evt.opcod = 'i';
      evt.p[1] *= -1;
    }


    if (opcod == 'f' && (int) evt.pcnt >= 2 && evt.p[2] <= FL(0.0)) {
      FUNC  *dummyftp;
      err = hfgens(csound, &dummyftp, &evt, 0);
    }
    else
      err = insert_score_event_at_sample(csound, &evt, csound->icurTime);
    if (UNLIKELY(err))
      csoundInitError(csound, Str("event_i: error creating '%c' event"),
                                opcod);
    return (err == 0 ? OK : NOTOK);
}

int eventOpcodeI(CSOUND *csound, LINEVENT *p)
{
    return eventOpcodeI_(csound, p, 0, 0);
}

int eventOpcodeI_S(CSOUND *csound, LINEVENT *p)
{
    return eventOpcodeI_(csound, p, 1, 0);
}

int instanceOpcode_(CSOUND *csound, LINEVENT2 *p, int insname)
{
    EVTBLK  evt;
    int     i;

    evt.strarg = NULL; evt.scnt = 0;
    evt.opcod = 'i';
    evt.pcnt = p->INOCOUNT;

       /* pass in the memory to hold the instance after insertion */
    evt.pinstance = (void *) p->inst;

    /* IV - Oct 31 2002: allow string argument */
    if (evt.pcnt > 0) {
      int res;
      if (insname) {
        res = strarg2insno(csound,
                                   ((STRINGDAT*) p->args[0])->data, 1);
        /* The comprison below and later is suspect */
        if (UNLIKELY(evt.p[1] == NOT_AN_INSTRUMENT)) return NOTOK;
        evt.p[1] = (MYFLT)res;
        evt.strarg = NULL; evt.scnt = 0;
      }
      else {
        if (isstrcod(*p->args[0])) {
          res = strarg2insno(csound,
                                     get_arg_string(csound, *p->args[0]), 1);
          if (UNLIKELY(evt.p[1] == NOT_AN_INSTRUMENT)) return NOTOK;
          evt.p[1] = (MYFLT)res;
        } else evt.p[1] = *p->args[0];
        evt.strarg = NULL; evt.scnt = 0;
      }
      for (i = 2; i <= evt.pcnt; i++)
        evt.p[i] = *p->args[i-1];
    }
    if (insert_score_event_at_sample(csound, &evt, csound->icurTime) != 0)
      return csoundPerfError(csound, &(p->h),
                               Str("instance: error creating event"));

    return OK;
}

int instanceOpcode(CSOUND *csound, LINEVENT2 *p)
{
    return instanceOpcode_(csound, p, 0);
}

int instanceOpcode_S(CSOUND *csound, LINEVENT2 *p)
{
    return instanceOpcode_(csound, p, 1);
}
