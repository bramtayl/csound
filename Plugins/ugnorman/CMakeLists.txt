make_plugin(ugnorman ugnorman.c)
target_link_libraries(ugnorman PRIVATE libcsound_internals)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()