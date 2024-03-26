make_utility(dnoise dnoise_main.c)
target_include_directories(dnoise PRIVATE ${Csound_SOURCE_DIR}/Plugins/stdutil)