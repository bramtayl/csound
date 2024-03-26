make_plugin(urandom urandom.c)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()