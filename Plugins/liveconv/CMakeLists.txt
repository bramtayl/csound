make_plugin(liveconv liveconv.c)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()