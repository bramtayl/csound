make_plugin(lufs lufs.c)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()