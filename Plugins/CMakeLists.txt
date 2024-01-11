##########################################
## Plugin opcodes are in the following categories ##
# 1) opcodes in C++ with no external dependencies
# 2) opcodes with special licensing conditions
# 3) platform-specific opcodes
# 4) opcodes with external dependencies
# 5) deprecated opcodes
#
#  if an opcode is not in one of the categories above,
#  it should be added with BUILTIN linkage
#  (i.e. not as plugins)
#  See instructions in ../Top/csmodule.c
##########################################

if(BUILD_CPP_OPCODES)
    add_subdirectory(ampmidid)
    add_subdirectory(arrayops)
    add_subdirectory(bformdec2)
    add_subdirectory(doppler)
    add_subdirectory(fractalnoise)
    add_subdirectory(ftsamplebank)
    add_subdirectory(lsfr)
    add_subdirectory(mixer)
    add_subdirectory(signalflowgraph)
    add_subdirectory(trigenvsegs)
endif()

if (UNIX)
    add_subdirectory(control)
    add_subdirectory(urandom)
endif()

add_subdirectory(cmidi)
add_subdirectory(deprecated)
add_subdirectory(dssi4cs)
add_subdirectory(ipmidi)
add_subdirectory(joystick)
add_subdirectory(osc)
add_subdirectory(padsynth)
add_subdirectory(pmidi)
add_subdirectory(rtalsa)
add_subdirectory(rtauhal)
add_subdirectory(rtcoreaudio)
add_subdirectory(rthaiku)
add_subdirectory(rtjack)
add_subdirectory(rtpa)
add_subdirectory(rtpulse)
add_subdirectory(rtwinmm)
add_subdirectory(scansyn)

