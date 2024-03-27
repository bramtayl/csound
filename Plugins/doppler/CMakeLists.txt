make_plugin(doppler doppler.cpp)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()