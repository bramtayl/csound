make_plugin(filter filter.c)
target_include_directories(filter PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()