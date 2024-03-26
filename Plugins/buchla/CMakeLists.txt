make_plugin(buchla buchla.c)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()