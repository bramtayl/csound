/*
    csdebug.c:

    Copyright (C) 2013 Andres Cabrera

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

#include <assert.h>

#include "csdebug.h"
#include "csdebug_internal.h"
#include "memalloc.h"

PUBLIC void csoundDebuggerInit(CSOUND *csound)
{
    csdebug_data_t *data =
      (csdebug_data_t *) mmalloc(csound, sizeof(csdebug_data_t));
    data->bkpt_anchor = (bkpt_node_t *) mmalloc(csound, sizeof(bkpt_node_t));
    data->bkpt_anchor->line = -1;
    data->bkpt_anchor->next = NULL;
    data->debug_instr_ptr = NULL;
    data->debug_opcode_ptr = NULL;
    data->bkpt_cb = NULL;
    data->status = CSDEBUG_STATUS_RUNNING;
    data->bkpt_buffer = csoundCreateCircularBuffer(csound,
                                                   64, sizeof(bkpt_node_t **));
    data->cmd_buffer = csoundCreateCircularBuffer(csound,
                                                  64, sizeof(debug_command_t));
    csound->csdebug_data = data;
    csound->kperf = kperf_debug;
}

PUBLIC void csoundDebuggerClean(CSOUND *csound)
{
    csdebug_data_t *data = (csdebug_data_t *) csound->csdebug_data;
    assert(data);
    bkpt_node_t *node = data->bkpt_anchor;
    csoundDestroyCircularBuffer(csound, data->bkpt_buffer);
    csoundDestroyCircularBuffer(csound, data->cmd_buffer);
    while (node) {
        bkpt_node_t *oldnode = node;
        node = node->next;
        mfree(csound, oldnode);
    }
    mfree(csound, data);
    csound->csdebug_data = NULL;
    csound->kperf = kperf_nodebug;
}

PUBLIC void csoundDebugStart(CSOUND *csound)
{
    csdebug_data_t *data = (csdebug_data_t *) csound->csdebug_data;
    data->status = CSDEBUG_STATUS_RUNNING;
}

PUBLIC void csoundSetBreakpoint(CSOUND *csound, int line, int instr, int skip)
{
    csdebug_data_t *data = (csdebug_data_t *) csound->csdebug_data;
    if (!data) {
      csoundWarning(csound,
                      Str("csoundSetBreakpoint: cannot set breakpoint. "
                          "Debugger is not initialised."));
      return;
    }
    if (line <= 0) {
      csoundWarning(csound, Str("csoundSetBreakpoint: line > 0 for breakpoint."));
      return;
    }
    bkpt_node_t *newpoint =
      (bkpt_node_t *) mmalloc(csound, sizeof(bkpt_node_t));
    newpoint->line = line;
    newpoint->instr = instr;
    newpoint->skip = skip;
    newpoint->count = skip;
    newpoint->mode = CSDEBUG_BKPT_LINE;
    csoundWriteCircularBuffer(csound, data->bkpt_buffer, &newpoint, 1);
}

PUBLIC void csoundRemoveBreakpoint(CSOUND *csound, int line, int instr)
{
    csdebug_data_t *data = (csdebug_data_t *) csound->csdebug_data;
    if (!data) {
      csoundWarning(csound,
                      Str("csoundRemoveBreakpoint: cannot remove breakpoint. "
                          "Debugger is not initialised."));
      return;
    }
    if (line < 0) {
      csoundWarning(csound, Str ("Negative line for breakpoint invalid."));
    }
    bkpt_node_t *newpoint =
      (bkpt_node_t *) mmalloc(csound, sizeof(bkpt_node_t));
    newpoint->line = line;
    newpoint->instr = instr;
    newpoint->mode = CSDEBUG_BKPT_DELETE;
    csoundWriteCircularBuffer(csound, data->bkpt_buffer, &newpoint, 1);
}

PUBLIC void csoundSetInstrumentBreakpoint(CSOUND *csound, MYFLT instr, int skip)
{
    csdebug_data_t *data = (csdebug_data_t *) csound->csdebug_data;
    if (!data) {
      csoundWarning(csound,
                      Str("csoundRemoveBreakpoint: cannot remove breakpoint. "
                          "Debugger is not initialised."));
      return;
    }
    assert(data);
    bkpt_node_t *newpoint =
      (bkpt_node_t *) mmalloc(csound, sizeof(bkpt_node_t));
    newpoint->line = -1;
    newpoint->instr = instr;
    newpoint->skip = skip;
    newpoint->count = skip;
    newpoint->mode = CSDEBUG_BKPT_INSTR;
    csoundWriteCircularBuffer(csound, data->bkpt_buffer, &newpoint, 1);
}

PUBLIC void csoundRemoveInstrumentBreakpoint(CSOUND *csound, MYFLT instr)
{
    csdebug_data_t *data = (csdebug_data_t *) csound->csdebug_data;
    assert(data);
    bkpt_node_t *newpoint =
      (bkpt_node_t *) mmalloc(csound, sizeof(bkpt_node_t));
    newpoint->line = -1;
    newpoint->instr = instr;
    newpoint->mode = CSDEBUG_BKPT_DELETE;
    csoundWriteCircularBuffer(csound, data->bkpt_buffer, &newpoint, 1);
}

PUBLIC void csoundClearBreakpoints(CSOUND *csound)
{
    csdebug_data_t *data = (csdebug_data_t *) csound->csdebug_data;
    assert(data);
    bkpt_node_t *newpoint =
      (bkpt_node_t *) mmalloc(csound, sizeof(bkpt_node_t));
    newpoint->line = -1;
    newpoint->instr = -1;
    newpoint->mode = CSDEBUG_BKPT_CLEAR_ALL;
    csoundWriteCircularBuffer(csound, data->bkpt_buffer, &newpoint, 1);
}

PUBLIC void csoundSetBreakpointCallback(CSOUND *csound,
                                       breakpoint_cb_t bkpt_cb, void *userdata)
{

    csdebug_data_t *data = (csdebug_data_t *) csound->csdebug_data;
    assert(data);
    data->bkpt_cb = bkpt_cb;
    data->cb_data = userdata;
}

PUBLIC void csoundDebugStepOver(CSOUND *csound)
{
    csdebug_data_t *data = (csdebug_data_t *) csound->csdebug_data;
    assert(data);
    debug_command_t command = CSDEBUG_CMD_STEPOVER;
    csoundWriteCircularBuffer(csound, data->cmd_buffer, &command, 1);
}

PUBLIC void csoundDebugStepInto(CSOUND *csound)
{
    csdebug_data_t *data = (csdebug_data_t *) csound->csdebug_data;
    assert(data);
    debug_command_t command = CSDEBUG_CMD_STEPINTO;
    csoundWriteCircularBuffer(csound, data->cmd_buffer, &command, 1);
}

PUBLIC void csoundDebugNext(CSOUND *csound)
{
    csdebug_data_t *data = (csdebug_data_t *) csound->csdebug_data;
    assert(data);
    debug_command_t command = CSDEBUG_CMD_NEXT;
    csoundWriteCircularBuffer(csound, data->cmd_buffer, &command, 1);
}

PUBLIC void csoundDebugContinue(CSOUND *csound)
{
    csdebug_data_t *data = (csdebug_data_t *) csound->csdebug_data;
    assert(data);
    debug_command_t command = CSDEBUG_CMD_CONTINUE;
    csoundWriteCircularBuffer(csound, data->cmd_buffer, &command, 1);
}

PUBLIC void csoundDebugStop(CSOUND *csound)
{
    csdebug_data_t *data = (csdebug_data_t *) csound->csdebug_data;
    assert(data);
    debug_command_t command = CSDEBUG_CMD_STOP;
    csoundWriteCircularBuffer(csound, data->cmd_buffer, &command, 1);
}

PUBLIC debug_instr_t *csoundDebugGetInstrInstances(CSOUND *csound)
{
    debug_instr_t *instrhead = NULL;
    debug_instr_t *debug_instr = NULL;
    INSDS *insds = csound->actanchor.nxtact;

    while (insds) {
        if (!instrhead) {
            instrhead = mmalloc(csound, sizeof(debug_instr_t));
            debug_instr = instrhead;
        } else {
            debug_instr->next = mmalloc(csound, sizeof(debug_instr_t));
            debug_instr = debug_instr->next;
        }
        debug_instr->lclbas = insds->lclbas;
        debug_instr->varPoolHead = insds->instr->varPool->head;
        debug_instr->instrptr = (void *) insds;
        debug_instr->p1 = insds->p1.value;
        debug_instr->p2 = insds->p2.value;
        debug_instr->p3 = insds->p3.value;
        debug_instr->kcounter = insds->kcounter;
        debug_instr->next = NULL;
        insds = insds->nxtact;
    }
    return instrhead;
}

PUBLIC void csoundDebugFreeInstrInstances(CSOUND *csound, debug_instr_t *instr)
{
    while (instr) {
        debug_instr_t *oldinstr = instr;
        instr = instr->next;
        mfree(csound, oldinstr);
    }
}

PUBLIC debug_variable_t *csoundDebugGetVariables(CSOUND *csound,
                                                 debug_instr_t *instr)
{
    debug_variable_t *head = NULL;
    debug_variable_t *debug_var = head;
    CS_VARIABLE * var = instr->varPoolHead;
    while (var) {
        void * varmem = NULL;
        if (!head) {
            head = mmalloc(csound, sizeof(debug_variable_t));
            debug_var = head;
        } else {
            debug_var->next = mmalloc(csound, sizeof(debug_variable_t));
            debug_var = debug_var->next;
        }
        debug_var->next = NULL;
        debug_var->name = var->varName;
        debug_var->typeName = var->varType->varTypeName;
        if (strcmp(debug_var->typeName, "i") == 0
                || strcmp(debug_var->typeName, "k") == 0
                || strcmp(debug_var->typeName, "a") == 0
                || strcmp(debug_var->typeName, "r") == 0
                ) {
            varmem = instr->lclbas + var->memBlockIndex;
        } else if (strcmp(debug_var->typeName, "S") == 0) {
            STRINGDAT *strdata = (STRINGDAT *) (instr->lclbas + var->memBlockIndex);
            varmem = &strdata->data[0];
        } else {
            csoundMessage(csound, "csoundDebugGetVarData() unknown data type.\n");
        }
        debug_var->data = varmem;
        var = var->next;
    }
    return head;
}


PUBLIC void csoundDebugFreeVariables(CSOUND *csound, debug_variable_t *varHead)
{
    while (varHead) {
        debug_variable_t *oldvar = varHead;
        varHead = varHead->next;
        mfree(csound, oldvar);
    }
}

