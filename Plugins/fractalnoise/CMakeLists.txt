make_plugin(fractalnoise fractalnoise.cpp libcsound_internals)
target_include_directories(fractalnoise PRIVATE ${libcsound_private_include_dirs})

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()