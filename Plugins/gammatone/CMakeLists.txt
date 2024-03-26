make_plugin(gammatone gammatone.c)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()