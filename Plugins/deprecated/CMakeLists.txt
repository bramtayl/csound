make_plugin(deprecated deprecated.c)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()