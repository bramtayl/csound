option(USE_IPMIDI "Build the IPMIDI I/O module" ON)

#check_deps(USE_IPMIDI PORTMIDI_HEADER PORTMIDI_LIBRARY)
if(USE_IPMIDI)
    if(WIN32)
      set(ipmidi_LIBS ws2_32)
    endif()
    make_plugin(ipmidi ipmidi.c "${ipmidi_LIBS}")
endif()
