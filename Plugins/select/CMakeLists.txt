make_plugin(select select.c)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()