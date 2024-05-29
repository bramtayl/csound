make_plugin(ugmoss ugmoss.c)
target_link_libraries(ugmoss PRIVATE libcsound_internals)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()