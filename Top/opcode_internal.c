#include "opcode_internal.h"
#include "opcode.h"
#include "text.h"
#include "interlocks.h"

void list_opcodes(CSOUND *csound, int level)
{
    opcodeListEntry *lst;
    const char      *sp = "                    ";   /* length should be 20 */
    int             j, k;
    int             cnt, len = 0, xlen = 0;
    int             count = 0;

    cnt = csoundNewOpcodeList(csound, &lst);
    if (UNLIKELY(cnt <= 0)) {
      csoundErrorMsg(csound, Str("Error creating opcode list"));
      csoundDisposeOpcodeList(csound, lst);
      return;
    }

    for (j = 0, k = -1; j < cnt; j++) {
      if ((level&1) == 0) {                         /* Print in 4 columns */
        if (j > 0 && strcmp(lst[j - 1].opname, lst[j].opname) == 0)
          continue;
        if ((level&2)==0 && ((lst[j].flags&_QQ) !=0)) {
          //printf("dropping %s\n", lst[j].opname);
          continue;
        }
        k++;
        xlen = 0;
        if (!(k & 3))
          csoundMessage(csound, "\n");
        else {
          if (len > 19) {
            xlen = len - 19;
            len = 19;
          }
          csoundMessage(csound, "%s", sp + len);
        }
        csoundMessage(csound, "%s", lst[j].opname);
        len = (int) strlen(lst[j].opname) + xlen;
      }
      else {
        char *ans = lst[j].outypes, *arg = lst[j].intypes;
        if ((level&2)==0 && ((lst[j].flags&_QQ) !=0)) {
          //printf("dropping %s\n", lst[j].opname);
          continue;
        }
        csoundMessage(csound, "%s", lst[j].opname);
        len = (int) strlen(lst[j].opname);
        if (len > 11) {
          xlen = len - 11;
          len = 11;
        }
        csoundMessage(csound, "%s", sp + (len + 8));
        if (ans == NULL || *ans == '\0') ans = "(null)";
        if (arg == NULL || *arg == '\0') arg = "(null)";
        csoundMessage(csound, "%s", ans);
        len = (int) strlen(ans) + xlen;
        len = (len < 11 ? len : 11);
        xlen = 0;
        csoundMessage(csound, "%s", sp + (len + 8));
        csoundMessage(csound, "%s\n", arg);
      }
      count++;
    }
    csoundMessage(csound, "\n");
    csoundMessage(csound, Str("%d opcodes\n\n"), count);
    csoundDisposeOpcodeList(csound, lst);
}
