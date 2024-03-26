make_utility(het_export hetx_main.c)
target_include_directories(het_export PRIVATE ${Csound_SOURCE_DIR}/Plugins/stdutil)