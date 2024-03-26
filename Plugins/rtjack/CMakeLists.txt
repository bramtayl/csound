set(rtjack_SRCS rtjack.c alphanumcmp.c)
make_plugin(rtjack "${rtjack_SRCS}" Jack::jack)
target_include_directories(rtjack PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})