make_plugin(metro metro.c)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()