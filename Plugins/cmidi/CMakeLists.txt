# option(USE_COREMIDI "Build the CoreMIDI I/O Module" ON)

if(APPLE)
    find_path(COREMIDI_INCLUDE_PATH CoreMIDI.h)
    find_library(COREMIDI_LIBRARY CoreMIDI)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)

    # check_deps(USE_COREMIDI APPLE COREMIDI_INCLUDE_PATH COREMIDI_LIBRARY COREFOUNDATION_LIBRARY)
    make_plugin(cmidi cmidi.c)
    target_include_directories(cmidi PRIVATE ${COREMIDI_INCLUDE_PATH})
    target_link_libraries(cmidi PRIVATE ${COREMIDI_LIBRARY} ${COREFOUNDATION_LIBRARY})
endif()
