make_plugin(padsynth padsynth_gen.cpp)
target_compile_features(padsynth PRIVATE cxx_std_11)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()