make_plugin(lfsr lfsr.cpp)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()