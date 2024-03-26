make_plugin(tabsum tabsum.c)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()