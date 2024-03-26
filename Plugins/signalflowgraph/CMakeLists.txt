make_plugin(signalflowgraph signalflowgraph.cpp)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()