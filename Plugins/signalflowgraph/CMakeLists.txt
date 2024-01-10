make_plugin(signalflowgraph signalflowgraph.cpp)
target_include_directories(signalflowgraph PRIVATE "${Csound_SOURCE_DIR}/Opcodes")
