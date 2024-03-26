make_plugin(serial serial.c)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()