set(sdif2ad_SRCS sdif2adsyn.c sdif.c sdif-mem.c)
make_utility(sdif2ad "${sdif2ad_SRCS}" "")
target_compile_definitions(sdif2ad PRIVATE "$<$<CONFIG:Release>:NDEBUG>") # needed for asserts
