# option(USE_AUDIOUNIT "Build the CoreAudio AudioUnit I/O module (requires CoreAudio)" ON)

if(APPLE)
    find_library(AUDIOUNIT_LIBRARY AudioUnit)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    find_path(COREAUDIO_INCLUDE_PATH CoreAudio.h)
    find_library(COREAUDIO_LIBRARY CoreAudio)

    # check_deps(USE_AUDIOUNIT APPLE COREAUDIO_INCLUDE_PATH COREAUDIO_LIBRARY AUDIOUNIT_INCLUDE_PATH AUDIOUNIT_LIBRARY COREFOUNDATION_LIBRARY)
    make_plugin(rtauhal rtauhal.c)
    target_include_directories(rtauhal PRIVATE ${AUDIOUNIT_INCLUDE_PATH})
    target_link_libraries(rtauhal PRIVATE ${AUDIOUNIT_LIBRARY} ${COREFOUNDATION_LIBRARY} ${COREAUDIO_LIBRARY})
endif()