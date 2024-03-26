make_plugin(dcblockr dcblockr.c)
target_include_directories(dcblockr PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()