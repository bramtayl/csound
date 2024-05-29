make_plugin(ugensa ugensa.c)
target_link_libraries(ugensa PRIVATE libcsound_internals)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()