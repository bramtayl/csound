if(WIN32)
    message(STATUS "Building Windows MME plugin(rtwinmm).")
    if(MSVC)
      make_plugin(rtwinmm rtwinmm.c)
      target_link_libraries(rtwinmm PRIVATE winmm.lib)
      target_link_libraries(rtwinmm PRIVATE gdi32)
      target_link_libraries(rtwinmm PRIVATE kernel32)
    else()
      set(rtwinmm_LIBS
          winmm gdi32 kernel32) # should probably do checks for these libs
      make_plugin(rtwinmm rtwinmm.c "${rtwinmm_LIBS}")
    endif()
endif()