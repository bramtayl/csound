make_plugin(joystick linuxjoystick.c)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()