make_plugin(pvsops pvsops.cpp)
target_compile_features(pvsops PRIVATE cxx_std_11)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()