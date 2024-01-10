option(BUILD_SCANSYN_OPCODES "Build the scansyn opcodes" ON)

if(BUILD_SCANSYN_OPCODES)
    set(scansyn_SRCS scansyn.c scansynx.c)
    make_plugin(scansyn "${scansyn_SRCS}" ${MATH_LIBRARY})
endif()
