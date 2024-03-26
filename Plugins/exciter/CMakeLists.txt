make_plugin(exciter exciter.c)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()