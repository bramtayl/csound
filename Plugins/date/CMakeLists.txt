make_plugin(date date.c)
if(BARE_METAL)
    target_compile_definitions(date PRIVATE "BARE_METAL")
endif()

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()