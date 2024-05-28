/*
 * File:   main.c
 * Author: stevenyi
 *
 * Created on June 7, 2012, 4:03 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "csoundCore_internal.h"
#include "csound_orc.h"
#include "gtest/gtest.h"
#include "memalloc.h"
#include "find_opcode.h"

extern "C" {
    extern OENTRIES* find_opcode2 (CSOUND* csound, char* opname);
    extern OENTRY* resolve_opcode (CSOUND*, OENTRIES* entries, char* outArgTypes, char* inArgTypes);

    extern bool check_in_arg (char* found, char* required);
    extern bool check_in_args (CSOUND* csound, char* outArgsFound, char* opOutArgs);
    extern bool check_out_arg (char* found, char* required);
    extern bool check_out_args (CSOUND* csound, char* outArgsFound, char* opOutArgs);
}

class OrcSemanticsTest : public ::testing::Test {
public:
    OrcSemanticsTest ()
    {
    }

    virtual ~OrcSemanticsTest ()
    {
    }

    virtual void SetUp ()
    {
        csoundSetGlobalEnv ("OPCODE6DIR64", "../../");
        csound = csoundCreate (0);
        csoundCreateMessageBuffer (csound, 0);
        csoundSetOption (csound, "--logfile=NULL");
    }

    virtual void TearDown ()
    {
        csoundCleanup (csound);
        csoundDestroyMessageBuffer (csound);
        csoundDestroy (csound);
        csound = nullptr;
    }

    CSOUND* csound {nullptr};
};

TEST_F (OrcSemanticsTest, FindOpcode2Test)
{
    OENTRIES* entries = find_opcode2(csound, (char*)"=");
    entries = find_opcode2(csound, (char*)"vco2");
    ASSERT_EQ (1, entries->count);
    mfree(csound, entries);
}

TEST_F (OrcSemanticsTest, ResolveOpcodeTest)
{
    OENTRIES* entries = find_opcode2(csound, (char*)"=");

    OENTRY* opc = resolve_opcode(csound, entries, (char*)"k", (char*)"k");
    ASSERT_TRUE (opc != NULL);
    mfree(csound, entries);

    entries = find_opcode2(csound, (char*)"vco2");
    ASSERT_EQ (1, entries->count);

    opc = resolve_opcode(csound, entries, (char*)"a", (char*)"cc");
    ASSERT_TRUE (opc != NULL);
    mfree(csound, entries);

    entries = find_opcode2(csound, (char*)"passign");
    ASSERT_EQ (3, entries->count);

    opc = resolve_opcode(csound, entries, (char*)"iiiiS", NULL);
    ASSERT_TRUE (opc != NULL);
    mfree(csound, entries);

    entries = find_opcode2(csound, (char*)"pcauchy");
    opc = resolve_opcode(csound, entries, (char*)"i", (char*)"k");
    ASSERT_TRUE (opc->iopadr != NULL);

    opc = resolve_opcode(csound, entries, (char*)"k", (char*)"k");
    ASSERT_TRUE (opc->kopadr != NULL);

    // TODO this test is failing
    // opc = resolve_opcode(csound, entries, "a", "k");
    // ASSERT_TRUE (opc->aopadr != NULL);

    mfree(csound, entries);
}

TEST_F (OrcSemanticsTest, FindOpcodeNewTest)
{
    ASSERT_TRUE (find_opcode_new(csound, (char*)"##error", (char*)"i", (char*)"i") != NULL);
    ASSERT_TRUE (find_opcode_new(csound, (char*)"##error", NULL, (char*)"i") == NULL);
    // TODO this assertion is failing
    // ASSERT_TRUE (find_opcode_new(csound, "##xin256", "i", NULL) != NULL);
    ASSERT_TRUE (find_opcode_new(csound, (char*)"##userOpcode", NULL, NULL) != NULL);
    ASSERT_TRUE (find_opcode_new(csound, (char*)"##array_set", NULL, (char*)"k[]k") != NULL);
    ASSERT_TRUE (find_opcode_new(csound, (char*)">=", (char*)"B", (char*)"kc") != NULL);
}

TEST_F (OrcSemanticsTest, CheckInArgsTest)
{
    ASSERT_FALSE (check_in_arg(NULL, NULL));
    ASSERT_FALSE (check_in_arg((char*)"a", NULL));
    ASSERT_FALSE (check_in_arg(NULL, (char*)"a"));
    ASSERT_TRUE (check_in_arg((char*)"a", (char*)"a"));
    ASSERT_FALSE (check_in_arg((char*)"a", (char*)"k"));
    ASSERT_TRUE (check_in_arg((char*)"c", (char*)"i"));
    ASSERT_TRUE (check_in_arg((char*)"i", (char*)"k"));

    // checking union types
    ASSERT_TRUE (check_in_arg((char*)"k", (char*)"x"));
    ASSERT_TRUE (check_in_arg((char*)"a", (char*)"x"));
    ASSERT_TRUE (check_in_arg((char*)"S", (char*)"T"));
    ASSERT_TRUE (check_in_arg((char*)"i", (char*)"T"));
    ASSERT_FALSE (check_in_arg((char*)"k", (char*)"T"));
    ASSERT_TRUE (check_in_arg((char*)"S", (char*)"U"));
    ASSERT_TRUE (check_in_arg((char*)"i", (char*)"U"));
    ASSERT_TRUE (check_in_arg((char*)"k", (char*)"U"));
    ASSERT_TRUE (check_in_arg((char*)"i", (char*)"k"));
    ASSERT_TRUE (check_in_arg((char*)"p", (char*)"k"));
    ASSERT_TRUE (check_in_arg((char*)"c", (char*)"k"));
    ASSERT_TRUE (check_in_arg((char*)"r", (char*)"k"));
    ASSERT_TRUE (check_in_arg((char*)"c", (char*)"i"));
    ASSERT_TRUE (check_in_arg((char*)"r", (char*)"k"));
    ASSERT_TRUE (check_in_arg((char*)"p", (char*)"k"));

    // checking var-arg types
    ASSERT_FALSE (check_in_arg((char*)"a", (char*)"m"));
    ASSERT_TRUE (check_in_arg((char*)"i", (char*)"m"));
    ASSERT_TRUE (check_in_arg((char*)"i", (char*)"M"));
    ASSERT_TRUE (check_in_arg((char*)"k", (char*)"M"));
    ASSERT_TRUE (check_in_arg((char*)"a", (char*)"M"));
    ASSERT_TRUE (check_in_arg((char*)"a", (char*)"N"));
    ASSERT_TRUE (check_in_arg((char*)"k", (char*)"N"));
    ASSERT_TRUE (check_in_arg((char*)"i", (char*)"N"));
    ASSERT_TRUE (check_in_arg((char*)"S", (char*)"N"));

    ASSERT_TRUE (check_in_arg((char*)"i", (char*)"n"));
    ASSERT_TRUE (check_in_arg((char*)"a", (char*)"y"));
    ASSERT_TRUE (check_in_arg((char*)"k", (char*)"z"));
    ASSERT_TRUE (check_in_arg((char*)"k", (char*)"Z"));
    ASSERT_TRUE (check_in_arg((char*)"a", (char*)"Z"));

    ASSERT_TRUE (check_in_arg((char*)"a", (char*)"."));
    ASSERT_TRUE (check_in_arg((char*)"k", (char*)"."));
    ASSERT_TRUE (check_in_arg((char*)"i", (char*)"."));

    ASSERT_TRUE (check_in_arg((char*)"a", (char*)"?"));
    ASSERT_TRUE (check_in_arg((char*)"k", (char*)"?"));
    ASSERT_TRUE (check_in_arg((char*)"i", (char*)"?"));

    ASSERT_TRUE (check_in_arg((char*)"a", (char*)"*"));
    ASSERT_TRUE (check_in_arg((char*)"k", (char*)"*"));
    ASSERT_TRUE (check_in_arg((char*)"i", (char*)"*"));

    //array
    ASSERT_FALSE (check_in_arg((char*)"a", (char*)"a[]"));
    ASSERT_FALSE (check_in_arg((char*)"a[]", (char*)"a"));
    ASSERT_TRUE (check_in_arg((char*)"a[]", (char*)"a[]"));
    ASSERT_FALSE (check_in_arg((char*)"k[]", (char*)"a[]"));
    ASSERT_TRUE (check_in_arg((char*)"a[]", (char*)"?[]"));
    ASSERT_TRUE (check_in_arg((char*)"k[]", (char*)"?[]"));
}

TEST_F (OrcSemanticsTest, CheckInArgs2Test)
{
    ASSERT_TRUE (check_in_args(csound, NULL, (char*)""));
    ASSERT_TRUE (check_in_args(csound, (char*)"", NULL));
    ASSERT_TRUE (check_in_args(csound, NULL, NULL));
    ASSERT_TRUE (check_in_args(csound, (char*)"", (char*)""));

    ASSERT_TRUE (check_in_args(csound, (char*)"akiSakiS", (char*)"N"));
    ASSERT_TRUE (check_in_args(csound, (char*)"akiSakiS", (char*)"aN"));
    ASSERT_FALSE (check_in_args(csound, (char*)"akiSakiS", (char*)"akiSakiSa"));

    ASSERT_TRUE (check_in_args(csound, (char*)"cc", (char*)"kkoM"));
    ASSERT_TRUE (check_in_args(csound, (char*)"k[]kk", (char*)".[].M"));
    ASSERT_TRUE (check_in_args(csound, (char*)"a", (char*)"az"));
}

TEST_F (OrcSemanticsTest, CheckOutArgTest)
{
    ASSERT_FALSE (check_out_arg(NULL, NULL));
    ASSERT_FALSE (check_out_arg((char*)"a", NULL));
    ASSERT_FALSE (check_out_arg(NULL, (char*)"a"));
    ASSERT_TRUE (check_out_arg((char*)"a", (char*)"a"));
    ASSERT_FALSE (check_out_arg((char*)"a", (char*)"k"));
    ASSERT_FALSE (check_out_arg((char*)"i", (char*)"k"));

    ASSERT_FALSE (check_out_arg((char*)"c", (char*)"i"));

    // checking union types
    ASSERT_TRUE (check_out_arg((char*)"k", (char*)"s"));
    ASSERT_TRUE (check_out_arg((char*)"a", (char*)"s"));
    ASSERT_TRUE (check_out_arg((char*)"p", (char*)"i"));

    // checking var-arg types
    ASSERT_TRUE (check_out_arg((char*)"a", (char*)"m"));
    ASSERT_TRUE (check_out_arg((char*)"k", (char*)"z"));
    ASSERT_TRUE (check_out_arg((char*)"i", (char*)"I"));
    ASSERT_TRUE (check_out_arg((char*)"a", (char*)"X"));
    ASSERT_TRUE (check_out_arg((char*)"k", (char*)"X"));
    ASSERT_TRUE (check_out_arg((char*)"i", (char*)"X"));
    ASSERT_FALSE (check_out_arg((char*)"S", (char*)"X"));
    ASSERT_TRUE (check_out_arg((char*)"a", (char*)"N"));
    ASSERT_TRUE (check_out_arg((char*)"k", (char*)"N"));
    ASSERT_TRUE (check_out_arg((char*)"i", (char*)"N"));
    ASSERT_TRUE (check_out_arg((char*)"S", (char*)"N"));
    ASSERT_TRUE (check_out_arg((char*)"f", (char*)"F"));

    //array
    ASSERT_FALSE (check_out_arg((char*)"a", (char*)"[a]"));
    ASSERT_FALSE (check_out_arg((char*)"a[]", (char*)"a"));
    ASSERT_TRUE (check_out_arg((char*)"a[]", (char*)"a[]"));
    ASSERT_FALSE (check_out_arg((char*)"k[]", (char*)"a[]"));
    ASSERT_TRUE (check_out_arg((char*)"a[]", (char*)".[]"));
}

TEST_F (OrcSemanticsTest, CheckOutArgs2Test)
{
    ASSERT_TRUE (check_out_args(csound, NULL, (char*)""));
    ASSERT_TRUE (check_out_args(csound, (char*)"", NULL));
    ASSERT_TRUE (check_out_args(csound, NULL, NULL));
    ASSERT_TRUE (check_out_args(csound, (char*)"", (char*)""));

    ASSERT_TRUE (check_out_args(csound, (char*)"akiSakiS", (char*)"N"));
    ASSERT_TRUE (check_out_args(csound, (char*)"akiSakiS", (char*)"aN"));
    ASSERT_FALSE (check_out_args(csound, (char*)"akiSakiS", (char*)"akiSakiSa"));

    ASSERT_TRUE (check_out_args(csound, (char*)"a", (char*)"aX"));
}
