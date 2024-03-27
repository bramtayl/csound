make_plugin(ampmidid ampmidid.cpp libcsound_internals)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()