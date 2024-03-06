make_plugin(ampmidid ampmidid.cpp libcsound_internals)
if(INIT_STATIC_MODULES)
    target_compile_defintions(ampmidid PRIVATE "INIT_STATIC_MODULES")
endif()