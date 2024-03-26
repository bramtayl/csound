make_plugin(signalflowgraph signalflowgraph.cpp libcsound_internals)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()