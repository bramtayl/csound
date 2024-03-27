set(dssi_SRC load.c dssi4cs.c)
make_plugin(dssi4cs "${dssi_SRC}" dl)

if(USE_LIB64)
    target_compile_definitions(dssi4cs PRIVATE "LIB64")
endif()

if(HAVE_STRLCAT)
    target_compile_definitions(dssi4cs PRIVATE "HAVE_STRLCAT")
endif()

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()