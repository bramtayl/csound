make_plugin(system_call system_call.c)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()