set(scansyn_SRCS scansyn.c scansynx.c)
make_plugin(scansyn "${scansyn_SRCS}" ${MATH_LIBRARY})

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()