make_plugin(signalflowgraph signalflowgraph.cpp libcsound_internals)
if(INIT_STATIC_MODULES)
    target_compile_defintions(signalflowgraph PRIVATE "INIT_STATIC_MODULES")
endif()