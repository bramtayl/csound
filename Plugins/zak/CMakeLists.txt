make_plugin(zak zak.c)
target_link_libraries(zak PRIVATE libcsound_internals)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()