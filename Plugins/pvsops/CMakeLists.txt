if(BUILD_CPP_OPCODES)
    make_plugin(pvsops pvsops.cpp)
    target_compile_features(pvsops PRIVATE cxx_std_11)
endif()