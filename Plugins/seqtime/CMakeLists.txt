make_plugin(seqtime seqtime.c)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()