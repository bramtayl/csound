make_plugin(squinewave squinewave.c)
target_link_libraries(squinewave PRIVATE libcsound_internals)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()