# UTILITY PLUGIN AND PROGRAMS

set(stdutil_SRCS
    atsa.c
    cvanal.c
    dnoise.c
    envext.c
    het_export.c
    het_import.c
    hetro.c
    lpanal.c
    lpc_export.c
    lpc_import.c
    mixer.c
    pvanal.c
    pv_export.c
    pv_import.c
    pvlook.c
    scale.c
    sndinfo.c
    srconv.c
    std_util.c
    xtrct.c
    ${Csound_SOURCE_DIR}/Executables/sdif2ad/sdif.c
)

make_plugin(stdutil "${stdutil_SRCS}" ${MATH_LIBRARY})
target_include_directories(stdutil PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Csound_SOURCE_DIR}/Executables/sdif2ad
)
