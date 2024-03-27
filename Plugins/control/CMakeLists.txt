make_plugin(control control.c)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()