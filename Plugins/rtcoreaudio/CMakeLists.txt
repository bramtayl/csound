# option(USE_COREAUDIO "Build the CoreAudio I/O module" ON)

# if(USE_COREAUDIO)
#    find_path(COREAUDIO_INCLUDE_PATH CoreAudio.h)
#    find_library(COREAUDIO_LIBRARY CoreAudio)
# endif()

# check_deps(USE_COREAUDIO APPLE COREAUDIO_INCLUDE_PATH COREAUDIO_LIBRARY)
# if(USE_COREAUDIO)
#    make_plugin(rtcoreaudio rtcoreaudio.c)
#    target_include_directories(rtcoreaudio PRIVATE ${COREAUDIO_INCLUDE_PATH})
#    target_link_libraries(rtcoreaudio ${COREAUDIO_LIBRARY})
# endif()
