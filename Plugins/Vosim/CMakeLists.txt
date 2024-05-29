make_plugin(Vosim Vosim.c)
target_link_libraries(Vosim PRIVATE libcsound_internals)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()