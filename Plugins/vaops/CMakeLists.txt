make_plugin(vaops vaops.c)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()