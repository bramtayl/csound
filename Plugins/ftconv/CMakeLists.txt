make_plugin(ftconv ftconv.c)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()