make_plugin(mixer mixer.cpp libcsound_internals)
if(INIT_STATIC_MODULES)
    target_compile_defintions(mixer PRIVATE "INIT_STATIC_MODULES")
endif()