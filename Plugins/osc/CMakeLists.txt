make_plugin(osc OSC.c)
target_link_libraries(osc PRIVATE LibLO::liblo Threads::Threads)
if(WIN32)
  target_link_libraries(osc PRIVATE wsock32 ws2_32 iphlpapi)
endif()

if(HAVE_UNISTD_H)
  target_compile_definitions(osc PRIVATE "HAVE_UNISTD_H")
endif()

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()