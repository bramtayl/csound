make_plugin(scoreline scoreline.c)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()