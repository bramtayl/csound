option(BUILD_DEPRECATED_OPCODES "Build deprecated opcodes" ON)

if(BUILD_DEPRECATED_OPCODES)
    make_plugin(deprecated deprecated.c)
endif()