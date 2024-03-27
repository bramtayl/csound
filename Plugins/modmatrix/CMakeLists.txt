make_plugin(modmatrix modmatrix.c)
target_include_directories(modmatrix PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()