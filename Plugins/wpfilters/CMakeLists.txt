make_plugin(wpfilters wpfilters.c)
target_link_libraries(wpfilters PRIVATE libcsound_internals)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()