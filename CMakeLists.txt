if (APPLE)
    cmake_minimum_required(VERSION 3.28)
else()
    cmake_minimum_required(VERSION 3.13.4)
endif()

cmake_policy(SET CMP0077 NEW)

include(cmake/cmake-utilities.cmake)

# silence RPATH cmake warning
set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_OSX_DEPLOYMENT_TARGET 11)

option(USE_VCPKG "Use VCPKG to download and manage dependencies" OFF)

if (USE_VCPKG)
    # Detect correct triplet based on CMake env
    if (CMAKE_SYSTEM_NAME STREQUAL "MinGW" AND UNIX)
        set(VCPKG_TARGET_TRIPLET "x64-mingw-static")
        set(VCPKG_INSTALL_OPTIONS "--allow-unsupported")
        set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/toolchains/mingw.cmake)
        set(VCPKG_CMAKE_SYSTEM_NAME MinGW)
        set(VCPKG_TARGET_ARCHITECTURE x64)
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Android")
        if (NOT ANDROID_ABI)
            message(FATAL_ERROR "Set ANDROID_ABI")
        endif()

        if (ANDROID_ABI STREQUAL "arm64-v8a")
            set(VCPKG_TARGET_TRIPLET "arm64-android")
        elseif (ANDROID_ABI STREQUAL "armeabi-v7a")
            set(VCPKG_TARGET_TRIPLET "arm-android")
        elseif (ANDROID_ABI STREQUAL "armeabi-v7a with NEON")
            set(VCPKG_TARGET_TRIPLET "arm-neon-android")
        elseif (ANDROID_ABI STREQUAL "x86_64")
            set(VCPKG_TARGET_TRIPLET "x64-android")
        elseif (ANDROID_ABI STREQUAL "x86")
            set(VCPKG_TARGET_TRIPLET "x86-android")
        else()
            message(FATAL_ERROR "Unrecognized ANDROID_ABI ${ANDROID_ABI}")
        endif()


        if (NOT CMAKE_ANDROID_NDK)
            message(FATAL_ERROR "Set CMAKE_ANDROID_NDK to the root path to NDK")
        endif()

        set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${CMAKE_ANDROID_NDK}/build/cmake/android.toolchain.cmake")
    elseif (WIN32)
        set(VCPKG_TARGET_TRIPLET "x64-windows-static")
    elseif(APPLE)
        set(VCPKG_TARGET_TRIPLET "x64-osx")
    elseif (UNIX)
        set(VCPKG_TARGET_TRIPLET "x64-linux")
    endif()

    set(VCPKG_FEATURE_FLAGS versions)
    set(X_VCPKG_APPLOCAL_DEPS_INSTALL ON)
    set(VCPKG_OVERLAY_TRIPLETS ${CMAKE_CURRENT_SOURCE_DIR}/platform/vcpkg/triplets)
    set(VCPKG_OVERLAY_PORTS ${CMAKE_CURRENT_SOURCE_DIR}/platform/vcpkg/ports)
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake
        CACHE STRING "Vcpkg toolchain file")
endif()

message(STATUS "VCPKG_TARGET_TRIPLET: ${VCPKG_TARGET_TRIPLET}")
message(STATUS "CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")

project(Csound)

enable_testing()

# for libcsound and plugins, utils, and tests that use internals
add_library(libcsound_internals INTERFACE)

# for users
add_library(libcsound_public_interface INTERFACE)

SET(BUILD_SHARED_LIBS ON)

if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
  set(CMAKE_COMPILER_IS_CLANG 1)
endif()

message(STATUS "Csound directory: ${CMAKE_HOME_DIRECTORY}")

# C++11 needed
target_compile_features(libcsound_public_interface INTERFACE cxx_std_11)

# Project definitions
set(APIVERSION "7.0")
set(CS_VERSION "7")
set(CS_SUBVER "0")
set(CS_PATCHLEVEL "0")

message(STATUS "Csound version: ${CS_VERSION}.${CS_SUBVER}.${CS_PATCHLEVEL}")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/README.md.in ${CMAKE_CURRENT_SOURCE_DIR}/README.md)

set(MACOSX_FRAMEWORK_BUNDLE_VERSION "${CS_VERSION}.${CS_SUBVER}.${CS_PATCHLEVEL}")
set(MACOSX_FRAMEWORK_SHORT_VERSION_STRING "${CS_VERSION}.${CS_SUBVER}.${CS_PATCHLEVEL}")
set(MACOSX_FRAMEWORK_IDENTIFIER "CSND")

# Relative install paths
set(EXECUTABLE_INSTALL_DIR "bin")
set(LOCALE_INSTALL_DIR "share/locale")
set(HEADER_INSTALL_DIR "include/csound")
set(SAMPLES_INSTALL_DIR "/samples")
set(CS_FRAMEWORK_DEST "$ENV{HOME}/Library/Frameworks" CACHE PATH "Csound framework path")

include(TestBigEndian)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckIncludeFileCXX)
include(CheckLibraryExists)
include(CMakeParseArguments)
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(CMakePushCheckState)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules/)
list(APPEND CMAKE_PREFIX_PATH /usr /usr/local /opt/local /sw)
message(STATUS "CMake module path: ${CMAKE_MODULE_PATH}")

if (CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
    execute_process(
        COMMAND brew --prefix bison
        RESULT_VARIABLE BREW_BISON
        OUTPUT_VARIABLE BREW_BISON_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (BREW_BISON EQUAL 0 AND EXISTS "${BREW_BISON_PREFIX}")
        message(STATUS "Found Bison keg installed by Homebrew at ${BREW_BISON_PREFIX}")
        set(BISON_EXECUTABLE "${BREW_BISON_PREFIX}/bin/bison")
    elseif(EXISTS "/usr/local/bin/bison")
        message(STATUS "Bison in /usr/local/bin")
        set(BISON_EXECUTABLE "/usr/local/bin/bison") 
    endif()
    
    execute_process(
        COMMAND brew --prefix flex
        RESULT_VARIABLE BREW_FLEX
        OUTPUT_VARIABLE BREW_FLEX_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (BREW_FLEX EQUAL 0 AND EXISTS "${BREW_FLEX_PREFIX}")
        message(STATUS "Found Flex keg installed by Homebrew at ${BREW_FLEX_PREFIX}")
        set(FLEX_EXECUTABLE "${BREW_FLEX_PREFIX}/bin/flex")
   elseif(EXISTS "/usr/local/bin/flex")
        message(STATUS "Flex in /usr/local/bin")
        set(FLEX_EXECUTABLE "/usr/local/bin/flex") 
    endif()
    
endif()

### COMPILER OPTIMIZATION FLAGS
option(USE_COMPILER_OPTIMIZATIONS "Use the default Csound compiler optimization flags" ON)
if(USE_COMPILER_OPTIMIZATIONS)
    include(cmake/CompilerOptimizations.cmake)
endif()

option(TRACK_COVERAGE "Track coverage" OFF)
if (TRACK_COVERAGE)
    include(cmake/coverage.cmake)
endif()

## USER OPTIONS ##
# Optional targets, they should all default to ON (check_deps will disable them if not possible to build)
option(USE_DOUBLE "Set to use double-precision floating point for audio samples." ON)
option(USE_LIBSNDFILE "Use libsndfile for sndfile IO, if available" ON)
option(BUILD_PERFTHREAD_CLASS "Include the Csound performance thread support (C++) " ON)
option(BUILD_UTILITIES "Build stand-alone executables for utilities that can also be used with -U" ON)
option(NEW_PARSER_DEBUG "Enable tracing of new parser" OFF)
option(BUILD_MULTI_CORE "Enable building for multicore system" ON)
option(FAIL_MISSING "Fail when a required external dependency is not present (useful for packagers)" OFF)
option(USE_GETTEXT "Use the Gettext internationalization library" ON)
option(BUILD_STATIC_LIBRARY "Also build a static version of the csound library" OFF)
option(USE_LRINT "Use lrint/lrintf for converting floating point values to integers." ON)
option(USE_CURL "Use CURL library" ON)
option(BUILD_RELEASE "Build for release" ON)
option(BUILD_INSTALLER "Build installer" OFF)
option(BUILD_TESTS "Build tests" ON)
option(LEXER_DEBUG "Build lexer with debug option" OFF)
option(USE_GIT_COMMIT "Show the git commit in version information" ON)
option(REQUIRE_PTHREADS "For non-Windows systems, set whether Csound will use threads or not" ON)
option(BUILD_DOCS "Build documentation (requires doxygen)" OFF)
option(DEBUG_ERROR_ON_WARNING "For a debug build, error on a compiler# warning" OFF)
option(BARE_METAL "To configure for bare metal build" OFF)
option(CUSTOM_MALLOC "Use custom malloc in bare metal builds" OFF)
option(INIT_STATIC_MODULES "Incorporate plugins into the library" OFF)
option(NO_SERIAL_OPCODES "No serial opcodes" OFF)
option(ADD_INTEGRATION_TESTS "Add command-line tests" ON)
option(ADD_MIDI_TESTS "Add MIDI tests" OFF)
option(ADD_MIC_TESTS "Add microphone tests" OFF)

if (BUILD_TESTS)
    # c tests require the static library
    set(BUILD_STATIC_LIBRARY ON)
endif()

if(BARE_METAL)
    target_compile_definitions(libcsound_internals INTERFACE "BARE_METAL")
    if(CUSTOM_MALLOC)
        target_compile_definitions(libcsound_internals INTERFACE "CUSTOM_MALLOC")
        # the default is for STM32H7: target_compile_definitions(libcsound_internals INTERFACE "MALLOC_BASE=0xC0000000")
    endif()
endif()

# Include this after the install path definitions so we can override them here.
# Also after function definitions so we can use them there
# Also after user options so that the default values don't overwrite the custom files values
find_file(CUSTOM_CMAKE "Custom.cmake" HINTS ${CMAKE_HOME_DIRECTORY})

if(CUSTOM_CMAKE)
    message(STATUS "Including Custom.cmake file: ${CUSTOM_CMAKE}")
    include(${CUSTOM_CMAKE})
else()
    message(STATUS "Not using Custom.cmake file.")
endif()

# in Release configuration, set NDEBUG
target_compile_definitions(libcsound_internals INTERFACE
    "$<$<CONFIG:Release>:NDEBUG>"
    "$<$<CONFIG:Debug>:BETA>"
)

if (NOT IOS)
    if (BARE_METAL)
        try_add_config_compile_option("Release" "-Os") # for smaller size
    else()
        try_add_config_compile_option("Release" "-O3")
    endif()
    try_add_config_compile_option("Debug" "-Wall")
    try_add_config_compile_option("Debug" "-Wno-missing-field-initializers")
    try_add_config_compile_option("Debug" "-Og") # debug optimization
    if (DEBUG_ERROR_ON_WARNING)
        try_add_config_compile_option("Debug" "-Werror")
    endif()
endif()

if(APPLE)
    #list(APPEND CMAKE_REQUIRED_INCLUDES "time.h")
    CHECK_FUNCTION_EXISTS(clock_gettime CLOCK_GETTIME)
    if(${CLOCK_GETTIME})
        message(STATUS  "clock_gettime() found")
        target_compile_definitions(libcsound_internals INTERFACE "HAVE_CLOCK_GETTIME")
    else()
        message(STATUS  "clock_gettime() not found")
    endif()
endif()

# library installation directories
option(USE_LIB64 "Set to on to set installation directory for libraries to lib64" OFF)
if(USE_LIB64)
	set(LIBRARY_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib64")
elseif(NOT DEFINED LIBRARY_INSTALL_DIR)
    set(LIBRARY_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif()
message(STATUS "LIBRARY INSTALL DIR: ${LIBRARY_INSTALL_DIR}")

if(USE_DOUBLE)
    message(STATUS "Building with 64-bit floats")
    set(PLUGIN_INSTALL_DIR "${LIBRARY_INSTALL_DIR}/csound/plugins64-${APIVERSION}")
else()
    message(STATUS "Building with 32-bit floats")
    set(PLUGIN_INSTALL_DIR "${LIBRARY_INSTALL_DIR}/csound/plugins-${APIVERSION}")
endif()

if(WIN32 AND NOT MSVC)
    if(EXISTS "C:/MinGW/include")
        target_include_directories(libcsound_internals INTERFACE "C:/MinGW/include")
    else()
        MESSAGE(STATUS "MinGW include dir not found.")
    endif()
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--add-stdcall-alias")
endif()




if(WIN32)
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
    set(CMAKE_SHARED_MODULE_PREFIX "")

    if (MSVC)
        target_link_libraries(libcsound_public_interface INTERFACE kernel32)
        target_link_libraries(libcsound_internals INTERFACE ws2_32)
    endif()

    if(MSVC)
        # Experimented with flags but did not make change performance results
        # try_add_compile_option("/Oi")
        # try_add_compile_option("/fp:fast")
        # try_add_compile_option("/arch:AVX2")
        # Replace the default CRT linkage from dynamic (MD) to static (MT)
        if(STATIC_CRT STREQUAL "ON")
            foreach(flag_var
                CMAKE_C_FLAGS CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_DEBUG
                CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
                CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
                string(REPLACE "/MD" "-MT" ${flag_var} "${${flag_var}}")
            endforeach()
            message(STATUS "Using static CRT linkage /MT")
        endif()
    else()
        try_add_compile_option("-mstackrealign")
        add_link_options("-static" "-static-libgcc" "-static-libstdc++")
    endif()
endif()

# MacOS/IOS c++11 flags
if(APPLE)
    set_target_properties(${CSOUNDLIB} PROPERTIES CXX_COMPILE_FLAGS  "-std=c++11")
endif()

set(BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})

try_add_compile_option("-fvisibility=hidden")
if(APPLE)
    if(IOS)
        message(STATUS "Building for iOS.")
    else()
        message(STATUS "Building for OSX")

        if(BUILD_INSTALLER)
            set(CS_FRAMEWORK_DEST "${CMAKE_INSTALL_PREFIX}")
        endif()

        set(DEFAULT_OSX_SYSROOT /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk/)
        if(NOT CMAKE_OSX_SYSROOT AND EXISTS ${DEFAULT_OSX_SYSROOT})
            set(CMAKE_OSX_SYSROOT ${DEFAULT_OSX_SYSROOT})
        endif()

        if(CMAKE_OSX_SYSROOT AND NOT CMAKE_OSX_SYSROOT MATCHES ".*OSX10.6.*"
            AND (NOT  CMAKE_OSX_DEPLOYMENT_TARGET))
            set(CMAKE_OSX_DEPLOYMENT_TARGET 10.7)
            message(STATUS "OSX: Setting Deployment Target to 10.7")
        else()
            message(STATUS "OSX: Setting Deployment Target to ${CMAKE_OSX_DEPLOYMENT_TARGET}")
        endif()

        target_compile_definitions(libcsound_internals INTERFACE "HAVE_VECLIB")
        if(NOT VECLIB_PATH)
            set(VECLIB_PATH "/System/Library/Frameworks/")
        endif()
    endif()
endif()

# Set plugins install directory
if(USE_DOUBLE)
    if(APPLE)
        set(CSOUNDLIB "CsoundLib64")
        set(PLUGIN_INSTALL_DIR "${CS_FRAMEWORK_DEST}/${CSOUNDLIB}.framework/Versions/${APIVERSION}/Resources/Opcodes64")
    else()
        set(CSOUNDLIB "csound64")
    endif()
else()
    if(APPLE)
        set(CSOUNDLIB "CsoundLib")
        set(PLUGIN_INSTALL_DIR "${CS_FRAMEWORK_DEST}/${CSOUNDLIB}.framework/Versions/${APIVERSION}/Resources/Opcodes")
    else()
        set(CSOUNDLIB "csound")
    endif()
endif()

# set default user plugin directory
if(APPLE)
    if(BUILD_INSTALLER)
       	set(CS_FRAMEWORK_FULL_PATH "/Library/Frameworks/${CSOUNDLIB}.framework/Versions/${APIVERSION}/Resources/Opcodes64")
    else()
        get_filename_component(CS_FRAMEWORK_FULL_PATH ${PLUGIN_INSTALL_DIR} ABSOLUTE)
    endif()

    add_definitions("-DCS_DEFAULT_PLUGINDIR=\"${CS_FRAMEWORK_FULL_PATH}\"")
    set(DEFAULT_OPCODEDIR ${CS_FRAMEWORK_FULL_PATH})

    # dir relative to $HOME
    if(USE_DOUBLE)
      set(DEFAULT_USER_PLUGINDIR "Library/csound/${APIVERSION}/plugins64")
   else()
      set(DEFAULT_USER_PLUGINDIR "Library/csound/${APIVERSION}/plugins")
    endif()

    target_compile_definitions(libcsound_internals INTERFACE "CS_DEFAULT_USER_PLUGINDIR=\"${DEFAULT_USER_PLUGINDIR}\"")
endif()

# LINUX definition
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(LINUX YES)
    message(STATUS "LINUX operating system found")
else()
    set(LINUX NO)
endif()

if(LINUX)
    set(DEFAULT_OPCODEDIR "${PLUGIN_INSTALL_DIR}")
    target_compile_definitions(libcsound_internals INTERFACE "CS_DEFAULT_PLUGINDIR=\"${DEFAULT_OPCODEDIR}\"")
    # dir relative to $HOME
    if(USE_DOUBLE)
        set(DEFAULT_USER_PLUGINDIR ".local/lib/csound/${APIVERSION}/plugins64")
    else()
        set(DEFAULT_USER_PLUGINDIR ".local/lib/csound/${APIVERSION}/plugins")
    endif()
    target_compile_definitions(libcsound_internals INTERFACE "CS_DEFAULT_USER_PLUGINDIR=\"${DEFAULT_USER_PLUGINDIR}\"")
endif()

if(WIN32)
    # dir relaive to %LOCALAPPDATA%
    if(USE_DOUBLE)
        set(DEFAULT_USER_PLUGINDIR "csound/${APIVERSION}/plugins64")
    else()
        set(DEFAULT_USER_PLUGINDIR "csound/${APIVERSION}/plugin")
    endif()
    target_compile_definitions(libcsound_internals INTERFACE "CS_DEFAULT_USER_PLUGINDIR=\"${DEFAULT_USER_PLUGINDIR}\"")
endif()

if(BUILD_RELEASE)
    target_compile_definitions(libcsound_internals INTERFACE "_CSOUND_RELEASE_")
else()
    target_compile_definitions(libcsound_internals INTERFACE "BETA")
endif()

if(APPLE OR LINUX)
    message(STATUS "default plugin dir set to ${DEFAULT_OPCODEDIR}")
endif()

set(BUILD_PLUGINS_DIR ${BUILD_DIR})
set(BUILD_BIN_DIR ${BUILD_DIR})
set(BUILD_LIB_DIR ${BUILD_DIR})

message(STATUS "BUILD_BIN_DIR set to ${BUILD_BIN_DIR}.")
message(STATUS "BUILD_LIB_DIR set to ${BUILD_LIB_DIR}.")
message(STATUS "BUILD_PLUGINS_DIR set to ${BUILD_PLUGINS_DIR}.")

# OS specific checks
TEST_BIG_ENDIAN(BIG_ENDIAN)

## HEADER/LIBRARY/OTHER CHECKS ##
# Check if libsndfile is available, if not, disable build
if(USE_LIBSNDFILE)
    if(USE_VCPKG)
        find_package(SndFile CONFIG)
        assign_bool(SndFile_FOUND TARGET SndFile::sndfile)
    else()
        find_package(SndFile MODULE)
    endif()
endif()
check_deps(USE_LIBSNDFILE SndFile_FOUND)

# carry on setting up libsndifle if so
if(USE_LIBSNDFILE)
    add_compile_definitions("USE_LIBSNDFILE")

    if(EMSCRIPTEN)
        target_include_directories(libcsound_internals INTERFACE ${Csound_SOURCE_DIR}/emscripten/deps/libsndfile-1.0.25/src/)
    endif()

    # add to required libraries so we can check for MIDI support
    list(APPEND CMAKE_REQUIRED_LIBRARIES SndFile::sndfile)

    check_c_source_compiles("#include <sndfile.h>
    int main(int argc, char** argv) {
    printf(\"SF_FORMAT_MPEG: %d\", SF_FORMAT_MPEG);
    return 0;
    }" HAVE_SF_FORMAT_MPEG)

    option(USE_MP3 "Use MP3 support in libsndfile" ON)
    check_deps(USE_MP3 HAVE_SF_FORMAT_MPEG)

    if (USE_MP3)
        message(STATUS  "Using libsndfile MP3 support")
        add_compile_definitions("SNDFILE_MP3")
    else()
        message(STATUS  "NOT Using libsndfile MP3 support")
    endif()
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads)
# windows threads aren't supported
assign_bool(NOT_WINDOWS_THREADS NOT CMAKE_USE_WIN32_THREADS_INIT)
check_deps(REQUIRE_PTHREADS Threads_FOUND NOT_WINDOWS_THREADS)

if(REQUIRE_PTHREADS)
    list(APPEND CMAKE_REQUIRED_LIBRARIES Threads::Threads)
endif()

# Now, non required library searches
option(USE_ATOMIC_BUILTIN "Use Atomic Builtins if supported" ON)

if(USE_ATOMIC_BUILTIN)
    include(cmake/CheckAtomic.cmake)
    if(HAVE_ATOMIC_BUILTIN)
        message(STATUS "Using atomic builtins.")
        target_compile_definitions(libcsound_public_interface INTERFACE "HAVE_ATOMIC_BUILTIN")
    else()
        message(STATUS "Not using atomic builtins - not found")
    endif()
else()
    message(STATUS "Not using atomic builtins - user disabled")
endif()

find_package(Intl)
find_package(Gettext)
find_library(MATH_LIBRARY m)

if (NOT MATH_LIBRARY)
    set(MATH_LIBRARY "")
endif()

cmake_push_check_state()
list(APPEND CMAKE_REQUIRED_LIBRARIES ${MATH_LIBRARY})
check_function_exists(lrint HAVE_LRINT)
cmake_pop_check_state()

# MSVC / VCPKG won't find dirent.h by default. Need to add the path explicitly
find_path(DIRENT_INCLUDE_DIR "dirent.h")
if (DIRENT_INCLUDE_DIR)
    target_include_directories(libcsound_internals INTERFACE ${DIRENT_INCLUDE_DIR})
    target_compile_definitions(libcsound_internals INTERFACE "HAVE_DIRENT_H")
else()
    if(MSVC)
        message(FATAL_ERROR "Dirent.h not found, Csound will not be able to load dynamic plugins. Cannot continue with project generation")
    else()
        message(WARNING "Dirent.h not found, Csound will not be able to load dynamic plugins such as real-time audio")
    endif()
endif()

set(HEADERS_TO_CHECK
    unistd.h io.h fcntl.h stdint.h
    sys/time.h sys/types.h termios.h
    values.h winsock.h sys/socket.h
    inttypes.h execinfo.h)

foreach(header ${HEADERS_TO_CHECK})
    # Convert to uppercase and replace [./] with _
    string(TOUPPER ${header} tmp)

    string(REGEX REPLACE [./] "_" upper_header ${tmp})
    check_include_file(${header} HAVE_${upper_header})
endforeach()

if (NOT HAVE_INTTYPES_H)
    message(FATAL_ERROR "Csound requires inttypes.h")
endif()

if (HAVE_EXECINFO_H)
    target_compile_definitions(libcsound_internals INTERFACE "HAVE_EXECINFO")
endif()

check_deps(USE_LRINT HAVE_LRINT)
if(USE_LRINT)
    target_compile_definitions(libcsound_public_interface INTERFACE "USE_LRINT")
endif()

## Check existence of CURL
find_package(CURL)
check_deps(USE_CURL CURL_FOUND)

if(USE_CURL)
    target_compile_definitions(libcsound_internals INTERFACE "HAVE_CURL")
    target_link_libraries(libcsound_internals INTERFACE CURL::libcurl)
endif()

# Flex/Bison for the new parser
find_package(FLEX)
find_package(BISON)

if(NOT FLEX_EXECUTABLE)
    message(FATAL_ERROR "Csound requires the flex executable")
endif()

if(NOT BISON_EXECUTABLE)
    message(FATAL_ERROR "Csound requires the bison executable")
endif()

## MAIN TARGETS ##

target_compile_definitions(libcsound_internals INTERFACE "__BUILDING_LIBCSOUND")

target_include_directories(libcsound_internals INTERFACE ${Csound_SOURCE_DIR}/H)
target_include_directories(libcsound_public_interface INTERFACE
    # the build interface needs an absolute path for public include dirs
    $<BUILD_INTERFACE:${Csound_SOURCE_DIR}/include>
     # the install interface needs a relative path
    $<INSTALL_INTERFACE:include>
)
target_include_directories(libcsound_internals INTERFACE 
    ${Csound_SOURCE_DIR}/Engine
    ${Csound_SOURCE_DIR}/Opcodes
    ${Csound_SOURCE_DIR}/util
)

#adding this for files that #include SDIF/sdif*
target_include_directories(libcsound_internals INTERFACE ${Csound_SOURCE_DIR}/util)
#adding this for files that #include files in local dirs
target_include_directories(libcsound_internals INTERFACE ${Csound_SOURCE_DIR})

#checking pthread functions
if(REQUIRE_PTHREADS)
  target_compile_definitions(libcsound_public_interface INTERFACE "HAVE_PTHREAD")

  if (NOT VCPKG_TARGET_TRIPLET STREQUAL "x64-mingw-static")
    check_function_exists(pthread_spin_lock PTHREAD_SPIN_LOCK_EXISTS)
  endif()

  check_function_exists(pthread_barrier_init PTHREAD_BARRIER_INIT_EXISTS)

  if(PTHREAD_SPIN_LOCK_EXISTS)
    target_compile_definitions(libcsound_public_interface INTERFACE "HAVE_PTHREAD_SPIN_LOCK")
  endif()

  if(PTHREAD_BARRIER_INIT_EXISTS)
    target_compile_definitions(libcsound_internals INTERFACE "HAVE_PTHREAD_BARRIER_INIT")
  endif()
endif()

# get the git hash and pass it to csound
set(git_hash_values "none")

if(USE_GIT_COMMIT)
    find_package(Git)
    message(STATUS "GIT: ${GIT_EXECUTABLE}")

    if(GIT_FOUND)
        execute_process(COMMAND ${GIT_EXECUTABLE} "rev-parse" "HEAD"
                        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                        OUTPUT_VARIABLE git_hash_value)

        if (git_hash_value)
            string(STRIP ${git_hash_value} git_hash_values)
            message(STATUS "HASH: ${git_hash_values}")
        endif()
    endif()
endif()

set_source_files_properties(Top/csound.c PROPERTIES COMPILE_FLAGS -DGIT_HASH_VALUE=${git_hash_values})

# The csound library
set(libcsound_SRCS
    Top/csound.c
    Engine/auxfd.c
    Engine/cfgvar.c
    Engine/corfiles.c
    Engine/entry1.c
    Engine/envvar.c
    Engine/envvar_public.c
    Engine/extract.c
    Engine/fgens_public.c
    Engine/insert.c
    Engine/insert_public.c
    Engine/linevent_public.c
    Engine/memalloc.c
    Engine/memfiles.c
    Engine/musmon.c
    Engine/namedins.c
    Engine/namedins_public.c
    Engine/rdscor.c
    Engine/scsort.c
    Engine/scxtract.c
    Engine/sort.c
    Engine/sread.c
    Engine/swritestr.c
    Engine/twarp.c
    Engine/csound_type_system.c
    Engine/csound_standard_types.c
    Engine/csound_data_structures.c
    Engine/pools.c
    Engine/ugen.c
    InOut/soundfile.c
    InOut/libsnd.c
    InOut/libsnd_u.c
    InOut/midifile.c
    InOut/midirecv.c
    InOut/midisend.c
    InOut/winascii.c
    InOut/windin.c
    InOut/cwindow.c
    InOut/winEPS.c
    InOut/circularbuffer.c
    OOps/aops.c
    OOps/aops_public.c
    OOps/bus.c
    OOps/cmath.c
    OOps/diskin2.c
    OOps/disprep.c
    OOps/dumpf.c
    OOps/fftlib.c
    OOps/lpred_public.c
    OOps/pffft.c
    OOps/goto_ops.c
    OOps/midiinterop.c
    OOps/midiops.c
    OOps/midiout.c
    OOps/mxfft.c
    OOps/oscils.c
    OOps/pstream.c
    OOps/pvfileio.c
    OOps/pvsanal.c
    OOps/random.c
    OOps/remote.c
    OOps/schedule.c
    OOps/sndinfUG.c
    OOps/str_ops.c
    OOps/ugens1.c
    OOps/ugens2.c
    OOps/ugens3.c
    OOps/ugens4.c
    OOps/ugens4_public.c
    OOps/ugens5.c
    OOps/ugens6.c
    OOps/ugtabs.c
    OOps/ugrw1.c
    OOps/vdelay.c
    OOps/compile_ops.c
    Opcodes/babo.c
    Opcodes/bilbar.c
    Opcodes/compress.c
    Opcodes/eqfil.c
    Opcodes/Vosim.c
    Opcodes/squinewave.c
    Opcodes/pinker.c
    Opcodes/pitch.c
    Opcodes/pitch0.c
    Opcodes/spectra.c
    Opcodes/sfont.c
    Opcodes/grain4.c
    Opcodes/hrtferX.c
    Opcodes/loscilx.c
    Opcodes/pan2.c
    Opcodes/arrays.c
    Opcodes/phisem.c
    Opcodes/hrtfopcodes.c
    Opcodes/vbap.c
    Opcodes/vbap1.c
    Opcodes/vbap_n.c
    Opcodes/vbap_zak.c
    Opcodes/harmon.c
    Opcodes/pitchtrack.c
    Opcodes/partikkel.c
    Opcodes/shape.c
    Opcodes/tabaudio.c
    Opcodes/crossfm.c
    Opcodes/pvlock.c
    Opcodes/modal4.c
    Opcodes/physutil.c
    Opcodes/physmod.c
    Opcodes/mandolin.c
    Opcodes/singwave.c
    Opcodes/fm4op.c
    Opcodes/moog1.c
    Opcodes/shaker.c
    Opcodes/bowedbar.c
    Opcodes/gab/tabmorph.c
    Opcodes/gab/hvs.c
    Opcodes/gab/sliderTable.c
    Opcodes/gab/newgabopc.c
    Opcodes/ftest.c
    Opcodes/hrtfearly.c
    Opcodes/hrtfreverb.c
    Opcodes/gendy.c
    Opcodes/tl/sc_noise.c
    Opcodes/afilters.c
    Opcodes/wpfilters.c
    Opcodes/zak.c
    Opcodes/fareygen.c
    Opcodes/quadbezier.c
    Opcodes/sequencer.c
    Opcodes/emugens/emugens.c
    Opcodes/emugens/scugens.c
    Top/argdecode.c
    Top/csdebug.c
    Top/csdebug_internal.c
    Top/cscore_internal.c
    Top/cscorfns.c
    Top/csmodule.c
    Top/csoundCore_internal.c
    Top/getstring.c
    Top/main.c
    Top/new_opts.c
    Top/one_file.c
    Top/opcode.c
    Top/threads.c
    Top/utility.c
    Top/threadsafe.c
    Top/server.c)

if(WIN32 AND NOT MSVC)
    set_source_files_properties(Opcodes/sfont.c PROPERTIES
        COMPILE_FLAGS -mno-ms-bitfields)
endif()

if(NOT MSVC)
    set_source_files_properties(Opcodes/sfont.c PROPERTIES
        COMPILE_FLAGS -Wno-address-of-packed-member)
endif()

set(stdopcod_SRCS
    Opcodes/bbcut.c
    Opcodes/biquad.c
    Opcodes/butter.c
    Opcodes/clfilt.c
    Opcodes/dam.c
    Opcodes/flanger.c
    Opcodes/follow.c
    Opcodes/fout.c
    Opcodes/ftgen.c
    Opcodes/gab/gab.c
    Opcodes/gab/vectorial.c
    Opcodes/grain.c
    Opcodes/locsig.c
    Opcodes/lowpassr.c
    Opcodes/midiops2.c
    Opcodes/midiops3.c
    Opcodes/newfils.c
    Opcodes/nlfilt.c
    Opcodes/oscbnk.c
    Opcodes/pluck.c
    Opcodes/paulstretch.c
    Opcodes/repluck.c
    Opcodes/reverbsc.c
    Opcodes/sndloop.c
    Opcodes/sndwarp.c
    Opcodes/space.c
    Opcodes/spat3d.c
    Opcodes/syncgrain.c
    Opcodes/ugens7.c
    Opcodes/ugens9.c
    Opcodes/ugensa.c
    Opcodes/uggab.c
    Opcodes/ugmoss.c
    Opcodes/ugnorman.c
    Opcodes/ugsc.c
    Opcodes/wave-terrain.c
    Opcodes/stdopcod.c
    Opcodes/socksend.c
    Opcodes/sockrecv.c)

set(cs_pvs_ops_SRCS
    Opcodes/ifd.c
    Opcodes/partials.c
    Opcodes/psynth.c
    Opcodes/pvsbasic.c
    Opcodes/pvscent.c
    Opcodes/pvsdemix.c
    Opcodes/pvs_ops.c
    Opcodes/pvsband.c
    Opcodes/pvsbuffer.c)

set(oldpvoc_SRCS
    Opcodes/dsputil.c
    Opcodes/pvadd.c
    Opcodes/pvinterp.c
    Opcodes/pvocext.c
    Opcodes/pvread.c
    Opcodes/ugens8.c
    Opcodes/vpvoc.c
    Opcodes/pvoc.c)

set(mp3in_SRCS
    Opcodes/mp3in.c
    InOut/libmpadec/layer1.c
    InOut/libmpadec/layer2.c
    InOut/libmpadec/layer3.c
    InOut/libmpadec/synth.c
    InOut/libmpadec/tables.c
    InOut/libmpadec/mpadec.c
    InOut/libmpadec/mp3dec.c)

list(APPEND libcsound_SRCS ${stdopcod_SRCS} ${cs_pvs_ops_SRCS} ${oldpvoc_SRCS} ${mp3in_SRCS})

if(BUILD_PERFTHREAD_CLASS)
    list(APPEND libcsound_SRCS "Top/csPerfThread.cpp")
endif()

if(INIT_STATIC_MODULES)
    set(static_modules_SRCS
        Top/init_static_modules.c
        Plugins/ampmidid/ampmidid.cpp
        Plugins/ambicode/ambicode.c
        Plugins/ambicode1/ambicode1.c
        Plugins/buchla/buchla.c
        Plugins/cellular/cellular.c
        Plugins/counter/counter.c
        Plugins/cpumeter/cpumeter.c
        Plugins/cross2/cross2.c
        Plugins/date/date.c
        Plugins/dcblockr/dcblockr.c
        Plugins/doppler/doppler.cpp
        Plugins/exciter/exciter.c
        Plugins/fareyseq/fareyseq.c
        Plugins/filter/filter.c
        Plugins/fractalnoise/fractalnoise.cpp
        Plugins/framebuffer/framebuffer.c
        Plugins/freeverb/freeverb.c
        Plugins/ftconv/ftconv.c
        Plugins/ftsamplebank/ftsamplebank.cpp
        Plugins/gammatone/gammatone.c
        Plugins/liveconv/liveconv.c
        Plugins/lufs/lufs.c
        Plugins/metro/metro.c
        Plugins/minmax/minmax.c
        Plugins/mixer/mixer.cpp
        Plugins/modmatrix/modmatrix.c
        Plugins/platerev/platerev.c
        Plugins/pvsgendy/pvsgendy.c
        Plugins/scoreline/scoreline.c
        Plugins/select/select.c 
        Plugins/seqtime/seqtime.c
        Plugins/signalflowgraph/signalflowgraph.cpp
        Plugins/sterrain/sterrain.c
        Plugins/system_call/system_call.c
        Plugins/tabsum/tabsum.c 
        Plugins/ugakbari/ugakbari.c 
        Plugins/vaops/vaops.c 
        Plugins/wterrain2/wterrain2.c
    )
    if (NOT WIN32)
        list(APPEND static_modules_SRCS Plugins/cpumeter/cpumeter.c)
    endif()
    if (NOT NO_SERIAL_OPCODES)
        list(APPEND static_modules_SRCS Plugins/serial/serial.c )
    endif()

    list(APPEND libcsound_SRCS ${static_modules_SRCS})
    list(APPEND libcsound_private_include_dirs
        Plugins/cross2
        Plugins/dcblockr
        Plugins/modmatrix
    )
    message(STATUS "Building static modules")
endif()

# Handling New Parser

set(YACC_SRC ${CMAKE_CURRENT_SOURCE_DIR}/Engine/csound_orc.y)
set(YACC_OUT ${CMAKE_CURRENT_BINARY_DIR}/Engine/csound_orcparse.c)
set(YACC_OUTH ${CMAKE_CURRENT_BINARY_DIR}/Engine/csound_orcparse.h)

set(LEX_SRC ${CMAKE_CURRENT_SOURCE_DIR}/Engine/csound_orc.lex)
set(LEX_OUT ${CMAKE_CURRENT_BINARY_DIR}/Engine/csound_orclex.c)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/Engine/csound_orclex.c
    PROPERTIES COMPILE_FLAGS "-Wno-implicit-fallthrough")

set(PRELEX_SRC ${CMAKE_CURRENT_SOURCE_DIR}/Engine/csound_pre.lex)
set(PRELEX_OUT ${CMAKE_CURRENT_BINARY_DIR}/Engine/csound_prelex.c)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/Engine/csound_prelex.c
    PROPERTIES COMPILE_FLAGS "-Wno-implicit-fallthrough")

if(LEXER_DEBUG)
    message(STATUS "Building lexer with debug option")

    add_custom_command(
        OUTPUT ${LEX_OUT}
        DEPENDS ${LEX_SRC}
        COMMAND ${FLEX_EXECUTABLE} ARGS -d -B -t ${LEX_SRC} > ${LEX_OUT}
    )
else()
    add_custom_command(
        OUTPUT ${LEX_OUT}
        DEPENDS ${LEX_SRC}
        COMMAND ${FLEX_EXECUTABLE} ARGS -B -t  ${LEX_SRC} > ${LEX_OUT}
    )
endif()

add_custom_command(
    DEPENDS ${PRELEX_SRC}
    COMMAND ${FLEX_EXECUTABLE} ARGS -B ${PRELEX_SRC} > ${PRELEX_OUT}
    OUTPUT ${PRELEX_OUT}
)

add_custom_command(
    OUTPUT ${YACC_OUT} ${YACC_OUTH}
    DEPENDS ${YACC_SRC} ${LEX_OUT}
    COMMAND ${BISON_EXECUTABLE}
    ARGS -pcsound_orc -d --report=itemset -o ${YACC_OUT} ${YACC_SRC}
)

list(APPEND libcsound_SRCS
    ${LEX_OUT} ${YACC_OUT} ${PRELEX_OUT}
    Engine/csound_orc_semantics.c
    Engine/csound_orc_semantics_public.c
    Engine/csound_orc_expressions.c
    Engine/csound_orc_optimize.c
    Engine/csound_orc_compile.c
    Engine/new_orc_parser.c
    Engine/symbtab.c)

set_source_files_properties(${YACC_OUT} GENERATED)
set_source_files_properties(${YACC_OUTH} GENERATED)
set_source_files_properties(${LEX_OUT} GENERATED)
set_source_files_properties(${PRELEX_OUT} GENERATED)

if(MSVC)
    set_source_files_properties(${LEX_OUT} PROPERTIES
        COMPILE_FLAGS -DYY_NO_UNISTD_H)
    set_source_files_properties(${PRELEX_OUT} PROPERTIES
        COMPILE_FLAGS -DYY_NO_UNISTD_H)
endif()

# Handling New Score Parser

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Engine)

##target_compile_definitions(libcsound_internals INTERFACE "SCORE_PARSER")

#set(YACC_SCOSRC ${CMAKE_CURRENT_SOURCE_DIR}/Engine/csound_sco.y)
#set(YACC_SCOOUT ${CMAKE_CURRENT_BINARY_DIR}/csound_scoparse.c)
#set(YACC_SCOOUTH ${CMAKE_CURRENT_BINARY_DIR}/csound_scoparse.h)

#set(LEX_SCOSRC ${CMAKE_CURRENT_SOURCE_DIR}/Engine/csound_sco.lex)
#set(LEX_SCOOUT ${CMAKE_CURRENT_BINARY_DIR}/csound_scolex.c)

set(PRELEX_SCOSRC ${CMAKE_CURRENT_SOURCE_DIR}/Engine/csound_prs.lex)
set(PRELEX_SCOOUT ${CMAKE_CURRENT_BINARY_DIR}/Engine/csound_prslex.c)

##message("${CMAKE_CURRENT_BINARY_DIR}/csound_prslex.c")
##message("${CMAKE_CURRENT_BINARY_DIR}/csound_scolex.c")
##message("${CMAKE_CURRENT_BINARY_DIR}/csound_scoparse.c")

#add_custom_command(
#    OUTPUT ${LEX_SCOOUT}
#    DEPENDS ${LEX_SCOSRC}
#    COMMAND ${FLEX_EXECUTABLE} ARGS -B -t ${LEX_SCOSRC} > ${LEX_SCOOUT}
#    )

add_custom_command(
    OUTPUT ${PRELEX_SCOOUT}
    DEPENDS ${PRELEX_SCOSRC}
    COMMAND ${FLEX_EXECUTABLE} ARGS -B -t -d ${PRELEX_SCOSRC} > ${PRELEX_SCOOUT}
)

#add_custom_command(
#    OUTPUT ${YACC_SCOOUT} ${YACC_SCOOUTH}
#    DEPENDS ${YACC_SCOSRC} ${LEX_SCOOUT}
#    COMMAND ${BISON_EXECUTABLE}
#    ARGS -pcsound_sco -t -d --report=itemset -o ${YACC_SCOOUT} ${YACC_SCOSRC}
#    )

list(APPEND libcsound_SRCS ${PRELEX_SCOOUT})
    #${LEX_SCOOUT} ${YACC_SCOOUT} ${PRELEX_SCOOUT})

#set_source_files_properties(${YACC_SCOOUT} GENERATED)
#set_source_files_properties(${LEX_SCOOUT} GENERATED)
set_source_files_properties(${PRELEX_SCOOUT} GENERATED)
if(MSVC)
    set_source_files_properties(${PRELEX_SCOOUT} PROPERTIES
        COMPILE_FLAGS -DYY_NO_UNISTD_H)
endif()

# for generated private headers
target_include_directories(libcsound_internals INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(libcsound_internals INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/Engine) # for csound_orcparse.h
target_include_directories(libcsound_internals INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/Frontends/beats) # for beats.tab.h
# for generated public headers
# not needed for install interface because generated headers get copied to the installed include
target_include_directories(libcsound_public_interface INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>)

if(NEW_PARSER_DEBUG)
    message(STATUS "Building with new parser debugging.")
    target_compile_definitions(libcsound_internals INTERFACE "PARSER_DEBUG=1")
else()
    message(STATUS "Not building with new parser debugging.")
endif()

if(BUILD_MULTI_CORE)
    message(STATUS "Building with multicore support.")

    list(APPEND libcsound_SRCS
        Engine/cs_new_dispatch.c
        Engine/cs_par_base.c
        Engine/cs_par_orc_semantic_analysis.c)

    target_compile_definitions(libcsound_internals INTERFACE "PARCS")

else()
    message(STATUS "Not building with multicore support.")
endif()

set(CSOUNDLIB_STATIC "${CSOUNDLIB}-static")

# ADDING HERE TO GRAB LIST OF HEADERS IN CASE OF BUILDING OSX FRAMEWORK
add_subdirectory(include)

if(APPLE)
    string(REGEX REPLACE "([^;]+)(;|$)" "include/\\1\\2" csheaders "${csheaders}")
    file(GLOB H_headers "${CMAKE_CURRENT_SOURCE_DIR}/H/*.h")
    set(libcsound_SRCS "${libcsound_SRCS};${csheaders};${H_headers};${CMAKE_CURRENT_BINARY_DIR}/include/float-version.h;${CMAKE_CURRENT_BINARY_DIR}/include/version.h")
endif()

if(NOT IOS)
    add_library(${CSOUNDLIB} SHARED ${libcsound_SRCS})
    target_link_libraries(${CSOUNDLIB} PRIVATE libcsound_internals)
    target_link_libraries(${CSOUNDLIB} PUBLIC libcsound_public_interface)
    set_target_properties(${CSOUNDLIB} PROPERTIES SOVERSION ${APIVERSION})
endif()

if(INIT_STATIC_MODULES)
    target_compile_definitions(libcsound_internals INTERFACE "INIT_STATIC_MODULES")
endif()

if(APPLE)
    if(NOT IOS)
        set_target_properties(${CSOUNDLIB} PROPERTIES FRAMEWORK YES)
        set_target_properties(${CSOUNDLIB} PROPERTIES FRAMEWORK_VERSION "${APIVERSION}")
        set_target_properties(${CSOUNDLIB} PROPERTIES PUBLIC_HEADER
            "${csheaders};${CMAKE_CURRENT_BINARY_DIR}/include/float-version.h;${CMAKE_CURRENT_BINARY_DIR}/include/version.h"
        )
    endif()
endif()

# Add the install target
install(TARGETS libcsound_public_interface EXPORT CsoundExports)

if(WIN32)
    install(TARGETS ${CSOUNDLIB}
        EXPORT CsoundExports
        RUNTIME DESTINATION "${EXECUTABLE_INSTALL_DIR}"
        ARCHIVE DESTINATION "${LIBRARY_INSTALL_DIR}"
        FRAMEWORK DESTINATION "${CS_FRAMEWORK_DEST}")
else()
    if(NOT IOS)
        install(TARGETS ${CSOUNDLIB}
            EXPORT CsoundExports
            LIBRARY DESTINATION "${LIBRARY_INSTALL_DIR}"
            ARCHIVE DESTINATION "${LIBRARY_INSTALL_DIR}"
            FRAMEWORK DESTINATION "${CS_FRAMEWORK_DEST}")
    endif()
endif()

if (LINUX)
    set_target_properties(${CSOUNDLIB} PROPERTIES
        INSTALL_RPATH "$ORIGIN"
    )
endif()

if(REQUIRE_PTHREADS)
    target_link_libraries(libcsound_internals INTERFACE Threads::Threads)
endif()

if (HAVE_ATOMIC_BUILTIN)
    target_link_libraries(libcsound_internals INTERFACE ${ATOMIC_LIB})
endif()

check_deps(USE_GETTEXT Intl_FOUND GETTEXT_FOUND)
if(USE_GETTEXT)
    message(STATUS "Using GNU gettext.")
    target_link_libraries(libcsound_internals INTERFACE Intl::Intl)
    target_compile_definitions(libcsound_internals INTERFACE "GNU_GETTEXT")
    if(NOT DEFINED CSOUND_TEXTDOMAIN AND BUILD_RELEASE AND (LINUX))
        file(TO_NATIVE_PATH "${CMAKE_INSTALL_PREFIX}/${LOCALE_INSTALL_DIR}" CSOUND_TEXTDOMAIN)
    endif()
    if(CSOUND_TEXTDOMAIN)
        message(STATUS "Setting textdomain to ${CSOUND_TEXTDOMAIN}.")
        set_source_files_properties(Top/getstring.c PROPERTIES
            COMPILE_FLAGS -DCSOUND_TEXTDOMAIN=\\\"${CSOUND_TEXTDOMAIN}\\\")
    else()
        message(STATUS "Not setting textdomain.")
    endif()
else()
    message(STATUS "Not using localization.")
endif()

if(LINUX)
    message(STATUS "Building on Linux.")
    target_compile_definitions(libcsound_internals INTERFACE "PIPES" "_GNU_SOURCE")
    target_link_libraries(libcsound_internals INTERFACE ${MATH_LIBRARY} dl)

    find_library(LIBRT_LIBRARY rt)

    if(LIBRT_LIBRARY)
        target_link_libraries(libcsound_internals INTERFACE ${LIBRT_LIBRARY})
        message(STATUS "  ADDING LIBRT LIBRARY: ${LIBRT_LIBRARY}.")
    endif()

    find_library(LIBEXECINFO_LIBRARY execinfo)

    if(LIBEXECINFO_LIBRARY)
        target_link_libraries(libcsound_internals INTERFACE ${LIBEXECINFO_LIBRARY})
        message(STATUS "  ADDING LIBEXECINFO LIBRARY: ${LIBEXECINFO_LIBRARY}.")
    endif()
endif()

if(APPLE AND NOT IOS)
    message(STATUS "Building on OSX")
    target_compile_definitions(libcsound_internals INTERFACE "PIPES")  

    find_package(Accelerate REQUIRED)
    target_link_libraries(libcsound_internals INTERFACE ${MATH_LIBRARY} dl Accelerate::accelerate)
endif()

if(WIN32)
    target_compile_definitions(libcsound_internals INTERFACE _WINSOCKAPI_) # disable winsock.h so we can use winsock2.h
endif()

check_function_exists(strlcat HAVE_STRLCAT)
if(HAVE_STRLCAT)
    target_compile_definitions(libcsound_public_interface INTERFACE "HAVE_STRLCAT")
endif()

# Locale-aware reading and printing
check_function_exists(strtok_r HAVE_STRTOK_R)
check_function_exists(strtod_l HAVE_STRTOD_L)
check_function_exists(sprintf_l HAVE_SPRINTF_L)

if(HAVE_STRTOK_R)
    target_compile_definitions(libcsound_internals INTERFACE "HAVE_STRTOK_R")
endif()
if(HAVE_STRTOD_L)
    target_compile_definitions(libcsound_internals INTERFACE "HAVE_STRTOD_L")
endif()
if(HAVE_SPRINTF_L)
    target_compile_definitions(libcsound_internals INTERFACE "HAVE_SPRINTF_L")
endif()

# Same for Windows
check_function_exists(_strtok_r HAVE__STRTOK_R)
check_function_exists(_strtod_l HAVE__STRTOD_L)
check_function_exists(_sprintf_l HAVE__SPRINTF_L)

# Pass flags according to system capabilities
# sockets aren't working in the xcframework
if((HAVE_WINSOCK_H OR HAVE_SYS_SOCKET_H) AND NOT IOS)
    target_compile_definitions(libcsound_internals INTERFACE "HAVE_SOCKETS")
endif()
if(HAVE_FCNTL_H)
    target_compile_definitions(libcsound_public_interface INTERFACE "HAVE_FCNTL_H")
endif()
if(HAVE_UNISTD_H)
    target_compile_definitions(libcsound_public_interface INTERFACE "HAVE_UNISTD_H")
endif()
if(HAVE_STDINT_H)
    target_compile_definitions(libcsound_public_interface INTERFACE "HAVE_STDINT_H")
endif()
if(HAVE_SYS_TIME_H)
    target_compile_definitions(libcsound_internals INTERFACE "HAVE_SYS_TIME_H")
endif()
if(HAVE_SYS_TYPES_H)
    target_compile_definitions(libcsound_public_interface INTERFACE "HAVE_SYS_TYPES_H")
endif()
if(HAVE_TERMIOS_H)
    target_compile_definitions(libcsound_internals INTERFACE "HAVE_TERMIOS_H")
endif()
if(HAVE_VALUES_H)
    target_compile_definitions(libcsound_internals INTERFACE "HAVE_VALUES_H")
endif()
# try_add_compile_option("-fno-strict-aliasing")

if (NO_SERIAL_OPCODES)
    list(APPEND libcsound_CFLAGS -DNO_SERIAL_OPCODES)
endif()

if(BIG_ENDIAN)
    target_compile_definitions(libcsound_internals INTERFACE "WORDS_BIGENDIAN")
endif()

if(USE_LIBSNDFILE)
    if (IOS)
        # TODO: figure out how to link the xcframework instead
        target_include_directories(libcsound_internals INTERFACE ${SndFile_INCLUDE_DIR})
    else()
        target_link_libraries(libcsound_public_interface INTERFACE SndFile::sndfile)
    endif()
endif()

if(NOT IOS)
    set_target_properties(${CSOUNDLIB} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${BUILD_BIN_DIR}
        LIBRARY_OUTPUT_DIRECTORY ${BUILD_LIB_DIR}
        ARCHIVE_OUTPUT_DIRECTORY ${BUILD_LIB_DIR})
endif()

if(BUILD_STATIC_LIBRARY)
    add_library(${CSOUNDLIB_STATIC} STATIC ${libcsound_SRCS})
    # BUILD_INTERFACE because to prevent the private interface from being exported
    target_link_libraries(${CSOUNDLIB_STATIC} PRIVATE $<BUILD_INTERFACE:libcsound_internals>)
    target_link_libraries(${CSOUNDLIB_STATIC} PUBLIC libcsound_public_interface)
    SET_TARGET_PROPERTIES(${CSOUNDLIB_STATIC} PROPERTIES OUTPUT_NAME ${CSOUNDLIB})
    SET_TARGET_PROPERTIES(${CSOUNDLIB_STATIC} PROPERTIES PREFIX "lib")

    if(MSVC)
        target_link_libraries(${CSOUNDLIB_STATIC} PRIVATE ${CSOUND_WINDOWS_LIBRARIES})
    else()
        target_link_libraries(${CSOUNDLIB_STATIC} PRIVATE ${libcsound_LIBS})
    endif()

    set_target_properties(${CSOUNDLIB_STATIC} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${BUILD_BIN_DIR}
        LIBRARY_OUTPUT_DIRECTORY ${BUILD_LIB_DIR}
        ARCHIVE_OUTPUT_DIRECTORY ${BUILD_LIB_DIR})

    if (LINUX)
        set_target_properties(${CSOUNDLIB_STATIC} PROPERTIES
            INSTALL_RPATH "$ORIGIN"
        )
    endif()

    # Add the install target
    install(TARGETS ${CSOUNDLIB_STATIC}
        EXPORT CsoundExports
        LIBRARY DESTINATION "${LIBRARY_INSTALL_DIR}"
        ARCHIVE DESTINATION "${LIBRARY_INSTALL_DIR}")
endif()

# used in both tests and util
option(BUILD_SRC_CONV "Build src_conv utility" ON)

if (BUILD_SRC_CONV)
    if (USE_VCPKG)
        find_package(SampleRate CONFIG REQUIRED)
        assign_bool(SampleRate_FOUND TARGET SampleRate::samplerate)
    else()
        find_package(SampleRate MODULE)
    endif()
endif()

check_deps(BUILD_SRC_CONV USE_LIBSNDFILE SampleRate_FOUND)

# needed for tests, plugin tests, and python
find_package(Python3)
check_deps(ADD_INTEGRATION_TESTS Python3_FOUND)

add_subdirectory(tests)

add_subdirectory(Executables)
add_subdirectory(Plugins)
add_subdirectory(po)

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

# target etags/tags
if(UNIX)
    add_custom_target(tags COMMAND etags `find ${CMAKE_CURRENT_SOURCE_DIR} -name \\*.cc -or -name \\*.hh -or -name \\*.cpp -or -name \\*.h -or -name \\*.c | grep -v " " `)
    add_custom_target(etags DEPENDS tags)
endif()

# build a CPack driven installer package
#include (InstallRequiredSystemLibraries)

set (CPACK_PACKAGE_NAME "csound")
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set (CPACK_PACKAGE_VERSION "6.0.0")
set (CPACK_PACKAGE_VERSION_MAJOR "6")
set (CPACK_PACKAGE_VERSION_MINOR "0")
set (CPACK_PACKAGE_VERSION_PATCH "0")
include (CPack)

# Documentation
if(BUILD_DOCS)
    find_package(Doxygen)

    if(DOXYGEN_FOUND)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.doxygen
            ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY
        )

        add_custom_target(doc ${DOXYGEN_EXECUTABLE}
            ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
    else()
        message(STATUS "Not building documentation")
    endif()
else()
    message(STATUS "Not building documentation")
endif()

# install CMake module
install(EXPORT CsoundExports
    FILE "CsoundConfig.cmake"
    NAMESPACE "Csound::"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/share/csound"
)

# install samples
if(APPLE)
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/samples
            DESTINATION  ${CS_FRAMEWORK_DEST}/${CSOUNDLIB}.framework/Versions/${APIVERSION})
else()
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/samples
            DESTINATION ${CMAKE_INSTALL_PREFIX}/share)
endif()

# install mingw libraries
if(MINGW)
    file(COPY /usr/lib/gcc/x86_64-w64-mingw32/10-posix/libstdc++-6.dll DESTINATION "${BUILD_DIR}")
    file(COPY /usr/lib/gcc/x86_64-w64-mingw32/10-posix/libgcc_s_seh-1.dll DESTINATION "${BUILD_DIR}")
    file(COPY /usr/x86_64-w64-mingw32/lib/libwinpthread-1.dll DESTINATION "${BUILD_DIR}")
endif()
