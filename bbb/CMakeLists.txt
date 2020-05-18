add_library(bbb include/bbb/packets.h include/bbb/badgerboard_datagrams.h
                include/bbb/badgerboard.h src/badgerboard.cc)

target_include_directories(bbb PUBLIC include)
target_include_directories(bbb PRIVATE include/bbb)

target_link_libraries(bbb PUBLIC util)
target_link_libraries(bbb PUBLIC nngpp)
