make_plugin(doppler doppler.cpp libcsound_internals)

if(INIT_STATIC_MODULES)
    target_compile_defintions(doppler PRIVATE "INIT_STATIC_MODULES")
endif()