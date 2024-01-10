option(BUILD_DSSI_OPCODES "Build the DSSI opcodes" OFF)

check_deps(BUILD_DSSI_OPCODES LINUX)
if(BUILD_DSSI_OPCODES)
    check_include_file(ladspa.h LADSPA_H)
    set(dssi_SRC load.c dssi4cs.c)
    make_plugin(dssi4cs "${dssi_SRC}" dl)
endif()