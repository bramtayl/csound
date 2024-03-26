make_plugin(arrayops arrayops.cpp)
target_compile_features(arrayops PRIVATE cxx_std_11)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()