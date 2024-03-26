make_plugin(cpumeter cpumeter.c)
if(CLOCK_GETTIME)
    target_compile_definitions(cpumeter PRIVATE "HAVE_CLOCK_GETTIME")
endif()

if (ADD_INTEGRATION_TESTS AND LINUX) # TODO: fix on macos
    add_subdirectory(tests)
endif()