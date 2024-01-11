option(USE_ALSA "Build the ALSA I/O module" ON)


if(USE_ALSA AND LINUX)
    if (USE_VCPKG)
        find_package(ALSA MODULE REQUIRED)
    else()
        find_package(ALSA REQUIRED)
    endif()

    make_plugin(rtalsa rtalsa.c ALSA::ALSA)
endif()
