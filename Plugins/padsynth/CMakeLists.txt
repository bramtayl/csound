option(BUILD_PADSYNTH_OPCODES "Build the Padsynth opcode" ON)

if(BUILD_PADSYNTH_OPCODES)
    make_plugin(padsynth padsynth_gen.cpp)
    target_compile_features(padsynth PRIVATE cxx_std_11)
endif()