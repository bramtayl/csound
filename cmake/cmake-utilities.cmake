# Utility function to make executables. All plugin targets should use this as it
# sets up output directory set in top-level CmakeLists.txt
# and adds an appropriate install target
#
# name - name of executable to produce
# srcs - list of src files
# libs - list of library files to link to
# output_name (OPTIONAL) - overide the name of the generated executable
#
function(make_executable name srcs libs)
    add_executable(${name} ${srcs})
    target_link_libraries (${name} PRIVATE ${libs})
    set_target_properties(${name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${BUILD_BIN_DIR})

    if(LINUX)
        set_target_properties(${name} PROPERTIES
            # back to install prefix from bin, then into lib
            INSTALL_RPATH "$ORIGIN/../lib"
        )
    endif()

    if(${ARGC} EQUAL 4)
        set_target_properties(${name} PROPERTIES
            OUTPUT_NAME ${ARGV3})
    endif()
    install(TARGETS ${name}
    EXPORT CsoundExports
	RUNTIME DESTINATION "${EXECUTABLE_INSTALL_DIR}" )
endfunction()

# Utility function to make a utility executable
#
# name - name of executable to produce
# srcs - list of src files
# libraries - OPTIONAL extra libraries to link
#
function(make_utility name srcs)
    make_executable(${name} "${srcs}" "${CSOUNDLIB}")
    set(i 2)
    while( ${i} LESS ${ARGC} )
        target_link_libraries(${name} PRIVATE ${ARGV${i}})
        math(EXPR i "${i}+1")
    endwhile()
endfunction()

# Checks if dependencies for an enabled target are fulfilled.
# If FAIL_MISSING is true and the dependencies are not fulfilled,
# it will abort the cmake run.
# If FAIL_MISSING is false, it will set the option to OFF.
# If the target is not enabled, it will do nothing.
# example: check_deps(BUILD_NEW_PARSER FLEX_EXECUTABLE
# BISON_EXECUTABLE)
#
function(check_deps option)
    if(${option})
        set(i 1)
        while( ${i} LESS ${ARGC} )
            set(dep ${ARGV${i}})
            if(NOT ${dep})
                if(FAIL_MISSING)
                    message(FATAL_ERROR
                        "${option} is enabled, but ${dep}=\"${${dep}}\"")
                else()
                    message(STATUS "${dep}=\"${${dep}}\", so disabling ${option}")
                    set(${option} OFF PARENT_SCOPE)
                    # Set it in the local scope too
                    set(${option} OFF)
                endif()
            endif()
            math(EXPR i "${i}+1")
        endwhile()
    endif()
    if(${option})
        message(STATUS "${option} is enabled.")
    else()
        message(STATUS "${option} is disabled.")
    endif()
endfunction()

# Utility function to make plugins. All plugin targets should use this as it
# sets up output directory set in top-level CmakeLists.txt
# and adds the appropriate install target
#
# libname - name of library to produce
# srcs - list of src files (must be quoted if a list)
# extralibs (OPTIONAL) - extra libraries to link the plugin to
#
# NB - this was moved here as it needs some VARS defined above
# for setting up the framework
function(make_plugin libname srcs)
    if(APPLE)
        add_library(${libname} SHARED ${srcs})
    else()
        add_library(${libname} MODULE ${srcs})
    endif()
    target_link_libraries(${libname} PUBLIC libcsound_public_interface)

    set(i 2)
    while( ${i} LESS ${ARGC} )
        target_link_libraries(${libname} PRIVATE ${ARGV${i}})
        math(EXPR i "${i}+1")
    endwhile()
  
    if (LINUX)
        set_target_properties(${libname} PROPERTIES
            # back to install prefix from plugins folder, then into lib
            INSTALL_RPATH "$ORIGIN/../../../lib"
        )
    endif()

    set_target_properties(${libname} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${BUILD_PLUGINS_DIR}
        LIBRARY_OUTPUT_DIRECTORY ${BUILD_PLUGINS_DIR}
        ARCHIVE_OUTPUT_DIRECTORY ${BUILD_PLUGINS_DIR})

    install(TARGETS ${libname}
        EXPORT CsoundExports
        LIBRARY DESTINATION "${PLUGIN_INSTALL_DIR}"
        ARCHIVE DESTINATION "${PLUGIN_INSTALL_DIR}" )
endfunction()

macro(assign_bool variable)
     if(${ARGN})
         set(${variable} ON)
     else()
         set(${variable} OFF)
     endif()
endmacro()

function(try_add_compile_option OPTION)
    check_c_compiler_flag(${OPTION} HAS_OPTION)
    check_cxx_compiler_flag(${OPTION} HAS_CXX_OPTION)
    if (HAS_OPTION)
        add_compile_options("$<$<COMPILE_LANGUAGE:C>:${OPTION}>")
    endif()
    if (HAS_CXX_OPTION)
        add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:${OPTION}>")
    endif()
endfunction()

function(try_add_config_compile_option OPTION_CONFIG OPTION)
    check_c_compiler_flag(${OPTION} HAS_OPTION)
    check_cxx_compiler_flag(${OPTION} HAS_CXX_OPTION)
    if (HAS_OPTION)
        add_compile_options("$<$<AND:$<COMPILE_LANGUAGE:C>,$<CONFIG:${OPTION_CONFIG}>>:${OPTION}>")
    endif()
    if (HAS_CXX_OPTION)
        add_compile_options("$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:${OPTION_CONFIG}>>:${OPTION}>")
    endif()
endfunction()

# Generate a python test from a python file
# You can use generator expressions in the python file, like "$<TARGET_FILE:csbeats>"
# You can import tools from test_helpers, for example:
# `from test_helpers import check_input, run_command, check_output`
function(generate_python_test test_name)
    file(GENERATE OUTPUT "$<TARGET_FILE_DIR:csound-bin>/${test_name}.py"
        INPUT "${test_name}.py.in"
    )

    add_test(
        NAME ${test_name}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ${Python3_EXECUTABLE}
            "$<TARGET_FILE_DIR:csound-bin>/${test_name}.py"
    )

    set_property(TEST ${test_name} PROPERTY LABELS ${ARGN})
endfunction()

# Add a csound test based on a .csd file
# in the same folder as the CMakeLists.txt file you call the function from 
# 
# Arguments:
# - filename: the filename of the .csd file
# - status: one of 
#     - "none": generates no output .wav file
#     - "indeterminate": generates an output .wav file that changes from run to run
#     - "consistent": generates a consistent output .wav file
# - labels: any number of labels for the test. You can use labels to filter which tests to run. e.g ctest -L my_label
# 
# so if filename is "array_copy", then the csd file will be "array_copy.csd"
# 
# There are three steps to running the tests:
# 1) Generate a .wav file from the csound file
#   If the status is "none", trying to generate a .wav file should yield no output. Otherwise:
# 
# 2) Calculate the checksum of the .wav file
#   Even a very small change in the .wav file will lead to a different checksum
# 
# 3a) If no checksum file exists, store the checksum
#   save the checksum in a new file in the same folder as the .csd file
#   If the filename is "array_copy", the checksum file will be "array_copy.wav.checksum"
#
# 3b) Else, compare the checksum to a stored checksum
#   If the status is "consistent", compare the new checksum to a previous saved checksum
#   If the checksums don't match, the test will fail
#   So if someone makes a change such that generated .wav files no longer sound right,
#   the tests will fail
# 
# If you want to regenerate all checksums, temporarily set REGENERATE_CHECKSUMS = True
# in the tests/integration.py.in file
function(add_integration_test test_name status)
    add_test(
        NAME ${test_name}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ${Python3_EXECUTABLE} "$<TARGET_FILE_DIR:csound-bin>/integration.py"
        ${test_name} ${status}
    )

    set_property(TEST ${test_name} PROPERTY LABELS ${ARGN})
endfunction()