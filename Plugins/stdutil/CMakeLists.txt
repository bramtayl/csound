# UTILITY PLUGIN AND PROGRAMS

set(executables_folder "${Csound_SOURCE_DIR}/Executables")

set(stdutil_SRCS
    ${executables_folder}/atsa/atsa.c
    ${executables_folder}/cvanal/cvanal.c
    ${executables_folder}/dnoise/dnoise.c
    ${executables_folder}/envext/envext.c
    ${executables_folder}/het_export/het_export.c
    ${executables_folder}/het_import/het_import.c
    ${executables_folder}/hetro/hetro.c
    ${executables_folder}/lpanal/lpanal.c
    ${executables_folder}/lpc_export/lpc_export.c
    ${executables_folder}/lpc_import/lpc_import.c
    ${executables_folder}/mixer-bin/mixer.c
    ${executables_folder}/pvanal/pvanal.c
    ${executables_folder}/pv_export/pv_export.c
    ${executables_folder}/pv_import/pv_import.c
    ${executables_folder}/pvlook/pvlook.c
    ${executables_folder}/scale/scale.c
    ${executables_folder}/sndinfo/sndinfo.c
    ${executables_folder}/srconv/srconv.c
    std_util.c
    ${executables_folder}/extractor/xtrct.c
    ${executables_folder}/sdif2ad/sdif.c
    ${Csound_SOURCE_DIR}/InOut/soundfile.c
)

if(BUILD_UTILITIES)
   make_plugin(stdutil "${stdutil_SRCS}" ${MATH_LIBRARY} ${LIBSNDFILE_LIBRARY} ${LIBSNDFILE_SUPPORT_LIBS})
   target_include_directories(stdutil PRIVATE ${executables_folder}/sdif2ad)
endif()
