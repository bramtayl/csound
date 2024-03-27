make_plugin(trigenvsegs trigEnvSegs.cpp)
target_compile_features(trigenvsegs PRIVATE cxx_std_11)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()