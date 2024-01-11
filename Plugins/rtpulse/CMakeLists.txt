option(USE_PULSEAUDIO "Build the PulseAudio I/O module" ON)

find_package(PulseAudio)
check_deps(USE_PULSEAUDIO PulseAudio_FOUND)
if(USE_PULSEAUDIO)
    make_plugin(rtpulse rtpulse.c PulseAudio::PulseAudio)
endif()