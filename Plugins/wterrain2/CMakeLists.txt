make_plugin(wterrain2 wterrain2.c)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()