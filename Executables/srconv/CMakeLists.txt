make_utility(srconv srconv_main.c)
target_include_directories(srconv PRIVATE ${Csound_SOURCE_DIR}/Plugins/stdutil)
