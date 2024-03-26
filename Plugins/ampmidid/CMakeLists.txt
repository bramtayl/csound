make_plugin(ampmidid ampmidid.cpp)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()