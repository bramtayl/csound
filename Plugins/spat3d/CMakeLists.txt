make_plugin(spat3d spat3d.c)
target_link_libraries(spat3d PRIVATE libcsound_internals)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()