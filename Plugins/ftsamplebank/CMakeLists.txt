make_plugin(ftsamplebank ftsamplebank.cpp)
target_include_directories(ftsamplebank PRIVATE ${DIRENT_INCLUDE_DIR})

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()