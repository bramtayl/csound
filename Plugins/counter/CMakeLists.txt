make_plugin(counter counter.c)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()