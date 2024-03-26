make_plugin(fareyseq fareyseq.c)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()