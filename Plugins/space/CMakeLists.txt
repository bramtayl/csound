make_plugin(space space.c)
target_link_libraries(space PRIVATE libcsound_internals)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()