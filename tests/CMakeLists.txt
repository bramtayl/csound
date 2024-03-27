add_subdirectory(c)

# test_helpers has three functions:
#   check_input: to check an input file
#   run_command: to run a command
#   check_output: to theck an output file
# generate it in two steps, so we can use both config-time and build-time variables

# To use config-time TEST_RUNNER
configure_file("test_helpers.py.in.in" "${CMAKE_BINARY_DIR}/test_helpers.py.in")

# To use build-time TARGET_FILE_DIR:csound-bin
file(GENERATE OUTPUT "$<TARGET_FILE_DIR:csound-bin>/test_helpers.py"
    INPUT "${CMAKE_BINARY_DIR}/test_helpers.py.in"
)

file(GENERATE OUTPUT "$<TARGET_FILE_DIR:csound-bin>/integration.py"
    INPUT "integration.py.in"
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_WRITE WORLD_EXECUTE SETUID SETGID
)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(commandline)
    add_subdirectory(Opcodes)
    add_subdirectory(regression)
    add_subdirectory(soak)
    add_subdirectory(Utilities)
endif()