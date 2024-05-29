make_plugin(tabaudio tabaudio.c)
target_link_libraries(tabaudio PRIVATE libcsound_internals)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()