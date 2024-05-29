make_plugin(ugsc ugsc.c)
target_link_libraries(ugsc PRIVATE libcsound_internals)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()