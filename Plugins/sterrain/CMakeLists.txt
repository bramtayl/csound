make_plugin(sterrain sterrain.c)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()