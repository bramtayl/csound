make_plugin(wave-terrain wave-terrain.c)
target_link_libraries(wave-terrain PRIVATE libcsound_internals)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()