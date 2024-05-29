make_plugin(ugens9 ugens9.c)
target_link_libraries(ugens9 PRIVATE libcsound_internals)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()