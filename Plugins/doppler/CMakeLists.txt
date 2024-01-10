make_plugin(doppler doppler.cpp)
target_include_directories(doppler PRIVATE "${Csound_SOURCE_DIR}/Opcodes")