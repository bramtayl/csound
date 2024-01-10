make_plugin(mixer mixer.cpp)
target_include_directories(mixer PRIVATE "${Csound_SOURCE_DIR}/Opcodes")
