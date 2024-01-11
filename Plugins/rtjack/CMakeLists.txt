option(USE_JACK "Build the jack I/O module and opcodes" ON)

if(USE_JACK)
    find_library(JACK_LIBRARY jack)
    check_include_file(jack/jack.h JACK_HEADER)
    if(APPLE)
    # VL: could not find a way to make check_include_file() work so
    # I am hardcoding it for MacOS
    set(JACK_HEADER  "jack/jack.h")
    endif()
    # HLO: including the jack2 common source dir
    # prevents "cannot open include file" errors
    find_path(JACK_INCLUDE_PATH jack/jack.h)
endif()

check_deps(USE_JACK JACK_HEADER JACK_LIBRARY JACK_INCLUDE_PATH Threads_FOUND)
if(USE_JACK)
    set(rtjack_LIBS ${JACK_LIBRARY} Threads::Threads)
    # set(rtjack_SRCS rtjack.c natsort.c)
    set(rtjack_SRCS rtjack.c alphanumcmp.c)
    make_plugin(rtjack "${rtjack_SRCS}" "${rtjack_LIBS}")
    # make_plugin(rtjack rtjack.c "${rtjack_LIBS}")
    target_include_directories(rtjack PRIVATE "${JACK_INCLUDE_PATH}")
endif()