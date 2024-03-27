make_plugin(mixer mixer.cpp)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()