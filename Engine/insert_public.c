#include "insert_public.h"

#include "csound_orc_semantics.h"
#include "insert.h"
#include "text.h"

int csoundInitError(CSOUND *csound, const char *s, ...) {
  va_list args;
  INSDS *ip;
  char buf[512];

  /* RWD: need this! */
  if (UNLIKELY(csound->ids == NULL)) {
    va_start(args, s);
    csoundErrMsgV(csound, Str("\nINIT ERROR: "), s, args);
    va_end(args);
    csoundLongJmp(csound, 1);
  }
  if (csound->mode != 1)
    csoundErrorMsg(csound, Str("InitError in wrong mode %d\n"), csound->mode);
  /* IV - Oct 16 2002: check for subinstr and user opcode */
  ip = csound->ids->insdshead;
  if (ip->opcod_iobufs) {
    OPCODINFO *op = ((OPCOD_IOBUFS *)ip->opcod_iobufs)->opcode_info;
    /* find top level instrument instance */
    do {
      ip = ((OPCOD_IOBUFS *)ip->opcod_iobufs)->parent_ip;
    } while (ip->opcod_iobufs);
    if (op)
      snprintf(buf, 512, Str("INIT ERROR in instr %d (opcode %s) line %d: "),
               ip->insno, op->name, csound->ids->optext->t.linenum);
    else
      snprintf(buf, 512, Str("INIT ERROR in instr %d (subinstr %d) line %d: "),
               ip->insno, csound->ids->insdshead->insno,
               csound->ids->optext->t.linenum);
  } else
    snprintf(buf, 512, Str("INIT ERROR in instr %d (opcode %s) line %d: "),
             ip->insno, csound->op, csound->ids->optext->t.linenum);
  va_start(args, s);
  csoundErrMsgV(csound, buf, s, args);
  va_end(args);
  do_baktrace(csound, csound->ids->optext->t.locn);
  putop(csound, &(csound->ids->optext->t));
  return ++(csound->inerrcnt);
}

int csoundPerfError(CSOUND *csound, OPDS *h, const char *s, ...) {
  va_list args;
  char buf[512];
  INSDS *ip = h->insdshead;
  TEXT t = h->optext->t;
  if (csound->mode != 2)
    csoundErrorMsg(csound, Str("PerfError in wrong mode %d\n"), csound->mode);
  if (ip->opcod_iobufs) {
    OPCODINFO *op = ((OPCOD_IOBUFS *)ip->opcod_iobufs)->opcode_info;
    /* find top level instrument instance */
    do {
      ip = ((OPCOD_IOBUFS *)ip->opcod_iobufs)->parent_ip;
    } while (ip->opcod_iobufs);
    if (op)
      snprintf(buf, 512, Str("PERF ERROR in instr %d (opcode %s) line %d: "),
               ip->insno, op->name, t.linenum);
    else
      snprintf(buf, 512, Str("PERF ERROR in instr %d (subinstr %d) line %d: "),
               ip->insno, ip->insno, t.linenum);
  } else
    snprintf(buf, 512, Str("PERF ERROR in instr %d (opcode %s) line %d: "),
             ip->insno, csound->op, t.linenum);
  va_start(args, s);
  csoundErrMsgV(csound, buf, s, args);
  va_end(args);
  do_baktrace(csound, t.locn);
  if (ip->pds) putop(csound, &(ip->pds->optext->t));
  csoundErrorMsg(csound, "%s", Str("   note aborted\n"));
  csound->perferrcnt++;
  xturnoff_now((CSOUND *)csound, ip); /* rm ins fr actlist */
  return csound->perferrcnt;          /* contin from there */
}