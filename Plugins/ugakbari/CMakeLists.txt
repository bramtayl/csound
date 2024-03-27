make_plugin(ugakbari ugakbari.c)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()