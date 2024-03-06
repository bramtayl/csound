make_plugin(bformdec2 bformdec2.cpp)
if(BIG_ENDIAN)
    target_compile_definitions(bformdec2 INTERFACE "WORDS_BIGENDIAN")
endif()