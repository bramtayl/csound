/*
    new_opts.c:

    Copyright (C) 2005 Istvan Varga

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

/**
 * Assignment to configuration variables from the command line can be done
 * with '-+NAME=VALUE'. Boolean variables can be set to true with any of
 * '-+NAME', '-+NAME=1', '-+NAME=yes', '-+NAME=on', and '-+NAME=true',
 * while setting to false is possible with any of '-+no-NAME', '-+NAME=0',
 *  '-+NAME=no', '-+NAME=off', and '-+NAME=false'.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "csoundCore_internal.h"
#include "csound.h"
#include "new_opts.h"
#include "memalloc.h"
#include "cfgvar.h"
#include "text.h"

/* list command line usage of all registered configuration variables */

void dump_cfg_variables(CSOUND *csound)
{
    csCfgVariable_t **p;
    int             i;

    p = csoundListConfigurationVariables(csound);
    if (p == NULL)
      return;
    if (p[0] == NULL)
      return;
    csoundMessage(csound, "\n");
    i = 0;
    do {
      csoundMessage(csound, "-+%s=", (char*) ((p[i])->h.name));
      switch ((p[i])->h.type) {
        case CSOUNDCFG_INTEGER:
          csoundMessage(csound, Str("<integer>"));
          if ((p[i])->i.min > -0x7FFFFFFF)
            csoundMessage(csound, ", %s%d", Str("min: "), (p[i])->i.min);
          if ((p[i])->i.max < 0x7FFFFFFF)
            csoundMessage(csound, ", %s%d", Str("max: "), (p[i])->i.max);
          if ((p[i])->i.flags & CSOUNDCFG_POWOFTWO)
            csoundMessage(csound, ", %s", Str("must be power of two"));
          break;
        case CSOUNDCFG_BOOLEAN:
          csoundMessage(csound, Str("<boolean>"));
          break;
        case CSOUNDCFG_FLOAT:
          csoundMessage(csound, Str("<float>"));
          if ((p[i])->f.min > -1.0e24f)
            csoundMessage(csound, ", %s%g", Str("min: "),
                                    (double) (p[i])->f.min);
          if ((p[i])->f.max < 1.0e24f)
            csoundMessage(csound, ", %s%g", Str("max: "),
                                    (double) (p[i])->f.max);
          break;
        case CSOUNDCFG_DOUBLE:
          csoundMessage(csound, Str("<float>"));
          if ((p[i])->d.min > -1.0e24)
            csoundMessage(csound, ", %s%g", Str("min: "),
                                    (double) (p[i])->d.min);
          if ((p[i])->d.max < 1.0e24)
            csoundMessage(csound, ", %s%g", Str("max: "),
                                    (double) (p[i])->d.max);
          break;
        case CSOUNDCFG_MYFLT:
          csoundMessage(csound, Str("<float>"));
          if ((p[i])->m.min > ((MYFLT) -1.0e24))
            csoundMessage(csound, ", %s%g", Str("min: "),
                                    (double) (p[i])->m.min);
          if ((p[i])->m.max < ((MYFLT) 1.0e24))
            csoundMessage(csound, ", %s%g", Str("max: "),
                                    (double) (p[i])->m.max);
          break;
        case CSOUNDCFG_STRING:
          csoundMessage(csound, Str("<string> (max. length = %d characters)"),
                                  (p[i])->s.maxlen - 1);
          break;
        default:
          csoundMessage(csound, Str("<unknown>"));
      }
      csoundMessage(csound, "\n");
      if ((p[i])->h.longDesc != NULL)
        csoundMessage(csound, "\t%s\n", Str((char*) p[i]->h.longDesc));
      else if ((p[i])->h.shortDesc != NULL)
        csoundMessage(csound, "\t%s\n", Str((char*) p[i]->h.shortDesc));
    } while (p[++i] != NULL);
}

/* Parse 's' as an assignment to a configuration variable in the format */
/* '-+NAME=VALUE'. In the case of boolean variables, the format may also */
/* be '-+NAME' for true, and '-+no-NAME' for false. */
/* Return value is zero on success. */

int parse_option_as_cfgvar(CSOUND *csound, const char *s)
{
    csCfgVariable_t *p;

    if (UNLIKELY((int) strlen(s) < 3)) {
      csoundWarning(csound, Str(" *** '%s' is not a valid "
                                  "Csound command line option."), s);
      csoundWarning(csound, Str(" *** Type 'csound --help' for the list of "
                                  "available options"));
      return 0;
    }
    if (UNLIKELY(strncmp(s, "-+", 2) != 0)) {
      csoundWarning(csound, Str(" *** '%s' is not a valid "
                                  "Csound command line option."), s);
      csoundWarning(csound, Str(" *** Type 'csound --help' for the list of "
                                  "available options"));
      return 0;
    }
    if (strchr(s, '=') == NULL) {
      /* there is no '=' character, must be a boolean */
      p = csoundQueryConfigurationVariable(csound, s + 2);
      if (p != NULL) {
        if (UNLIKELY(p->h.type != CSOUNDCFG_BOOLEAN)) {
          csoundWarning(csound, Str(" *** type of option '%s' "
                                      "is not boolean"), s + 2);
          return 0;
        }
        *(p->b.p) = 1;
      }
      else if (LIKELY((int) strlen(s) > 5)) {
        if (UNLIKELY(strncmp(s, "-+no-", 5) != 0)) {
          csoundWarning(csound, Str(" *** '%s': invalid option name"),
                                  s + 2);
          return 0;
        }
        p = csoundQueryConfigurationVariable(csound, s + 5);
        if (UNLIKELY(p == NULL)) {
          csoundWarning(csound, Str(" *** '%s': invalid option name"),
                                  s + 2);
          return -1;
        }
        if (UNLIKELY(p->h.type != CSOUNDCFG_BOOLEAN)) {
          csoundWarning(csound, Str(" *** type of option '%s' "
                                      "is not boolean"), s + 2);
          return 0;
        }
        *(p->b.p) = 0;
      }
      else {
        csoundWarning(csound, Str(" *** '%s': invalid option name"), s + 2);
        return 0;
      }
    }
    else if (LIKELY((int) strlen(s) > 3)) {
      char *buf, *val, *tmp;
      int  retval;
      buf = (char*) mmalloc(csound,
                                   sizeof(char) * (size_t) ((int) strlen(s) - 1));
      if (UNLIKELY(buf == NULL)) {
        csoundWarning(csound, Str(" *** memory allocation failure"));
        return -1;
      }
      /* strcpy(buf, s + 2); */
      val = (char*) s+2;
      tmp = buf;
      while (*val!='\0') {
        /*
         * CAN char used during the parsing in CsOptions to mark
         * the removable characters '\'. ETX char used to mark
         * the limits of a string.
         */
        if (*val != 0x18 && *val != 3)
          *tmp++ = *val;
        val++;
      }
      *tmp='\0';
      val = strchr(buf, '=');
      *(val++) = '\0';  /* 'buf' is now the name, 'val' is the value string */
      retval = csoundParseConfigurationVariable(csound, buf, val);
      if (UNLIKELY(retval != CSOUNDCFG_SUCCESS)) {
        csoundWarning(csound,
                        Str(" *** error setting option '%s' to '%s': %s"),
                        buf, val, csoundCfgErrorCodeToString(retval));
        mfree(csound, (void*) buf);
        return 0;
      }
      mfree(csound, (void*) buf);
    }
    else {
      csoundWarning(csound, Str(" *** '%s' is not a valid "
                                  "Csound command line option."), s);
      csoundWarning(csound, Str(" *** Type 'csound --help' for the list of "
                                  "available options."));
      return 0;
    }
    return 0;
}
