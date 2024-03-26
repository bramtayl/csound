make_plugin(platerev platerev.c)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()