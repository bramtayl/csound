make_plugin(syncgrain syncgrain.c)
target_link_libraries(syncgrain PRIVATE libcsound_internals)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()