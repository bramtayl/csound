make_plugin(ambicode ambicode.c)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()