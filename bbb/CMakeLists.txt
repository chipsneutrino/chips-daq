add_library(bbb include/bbb/packets.h include/bbb/dummy.h src/dummy.cc)

target_include_directories(bbb PUBLIC include)
target_include_directories(bbb PRIVATE include/bbb)

target_link_libraries(bbb PUBLIC util)
