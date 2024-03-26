make_plugin(freeverb freeverb.c)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()