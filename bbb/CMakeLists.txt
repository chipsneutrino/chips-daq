add_library(
  bbb
  include/bbb/packets.h
  include/bbb/badgerboard_datagrams.h
  include/bbb/badgerboard_state.h
  include/bbb/badgerboard_control.h
  src/badgerboard_control.cc
  include/bbb/constants.h
  include/bbb/badgerboard_heartbeat.h
  src/badgerboard_heartbeat.cc)

target_include_directories(bbb PUBLIC include)
target_include_directories(bbb PRIVATE include/bbb)

target_link_libraries(bbb PUBLIC util)
target_link_libraries(bbb PUBLIC nngpp)
