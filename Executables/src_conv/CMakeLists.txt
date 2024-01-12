if(USE_VCPKG)
    set(LIBSNDFILE_LIBRARY SndFile::sndfile)
endif()


if (USE_VCPKG)
    find_package(SampleRate REQUIRED)
    set(LIBSAMPLERATE_LIBRARY SampleRate::samplerate)
else ()
    find_library(LIBSAMPLERATE_LIBRARY NAMES samplerate libsamplerate-0)
endif()

if(USE_LIBSNDFILE AND LIBSAMPLERATE_LIBRARY)
    make_executable(src_conv new_srconv.c "${LIBSNDFILE_LIBRARY};${LIBSAMPLERATE_LIBRARY}")
else()
    message(STATUS "Not building src_conv (libsndfile or libsamplerate not found).")
endif()