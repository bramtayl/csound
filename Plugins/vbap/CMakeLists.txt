set(VBAP_SOURCES vbap_n.c vbap_zak.c vbap.c vbap1.c)
make_plugin(vbap "${VBAP_SOURCES}")
target_link_libraries(vbap PRIVATE libcsound_internals)

if (ADD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()