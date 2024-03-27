make_plugin(minmax minmax.c)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()